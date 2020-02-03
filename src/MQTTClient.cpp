#include "MQTTClient.h"

inline void lwmqtt_arduino_timer_set(void *ref, uint32_t timeout) {
  // cast timer reference
  auto t = (lwmqtt_arduino_timer_t *)ref;

  // set future end time
  if (t->millis != nullptr) {
    t->end = (uint32_t)(t->millis() + timeout);
  } else {
    t->end = (uint32_t)(millis() + timeout);
  }
}

inline int32_t lwmqtt_arduino_timer_get(void *ref) {
  // cast timer reference
  auto t = (lwmqtt_arduino_timer_t *)ref;

  // get difference to end time
  if (t->millis != nullptr) {
    return (int32_t)(t->end - t->millis());
  } else {
    return (int32_t)(t->end - millis());
  }
}

inline lwmqtt_err_t lwmqtt_arduino_network_read(void *ref, uint8_t *buffer, size_t len, size_t *read,
                                                uint32_t timeout) {
  // cast network reference
  auto n = (lwmqtt_arduino_network_t *)ref;

  // set timeout
  uint32_t start = millis();

  // reset counter
  *read = 0;

  // read until all bytes have been read or timeout has been reached
  while (len > 0 && (millis() - start < timeout)) {
    // read from connection
    int r = n->client->read(buffer, len);

    // handle read data
    if (r > 0) {
      buffer += r;
      *read += r;
      len -= r;
      continue;
    }

    // wait/unblock for some time (RTOS based boards may otherwise fail since
    // the wifi task cannot provide the data)
    delay(0);

    // otherwise check status
    if (!n->client->connected()) {
      return LWMQTT_NETWORK_FAILED_READ;
    }
  }

  // check counter
  if (*read == 0) {
    return LWMQTT_NETWORK_TIMEOUT;
  }

  return LWMQTT_SUCCESS;
}

inline lwmqtt_err_t lwmqtt_arduino_network_write(void *ref, uint8_t *buffer, size_t len, size_t *sent,
                                                 uint32_t /*timeout*/) {
  // cast network reference
  auto n = (lwmqtt_arduino_network_t *)ref;

  // write bytes
  *sent = n->client->write(buffer, len);
  if (*sent <= 0) {
    return LWMQTT_NETWORK_FAILED_WRITE;
  }

  return LWMQTT_SUCCESS;
}

static void MQTTClientHandler(lwmqtt_client_t * /*client*/, void *ref, lwmqtt_string_t topic,
                              lwmqtt_message_t message) {
  // get callback
  auto cb = (MQTTClientCallback *)ref;

  // null terminate topic
  char terminated_topic[topic.len + 1];
  memcpy(terminated_topic, topic.data, topic.len);
  terminated_topic[topic.len] = '\0';

  // null terminate payload if available
  if (message.payload != nullptr) {
    message.payload[message.payload_len] = '\0';
  }

  // call the advanced callback and return if available
  if (cb->advanced != nullptr) {
    cb->advanced(cb->client, terminated_topic, (char *)message.payload, (int)message.payload_len);
    return;
  }

  // return if simple callback is not set
  if (cb->simple == nullptr) {
    return;
  }

  // create topic string
  String str_topic = String(terminated_topic);

  // create payload string
  String str_payload;
  if (message.payload != nullptr) {
    str_payload = String((const char *)message.payload);
  }

  // call simple callback
  cb->simple(str_topic, str_payload);
}

MQTTClient::MQTTClient(int bufSize) {
  // reset client
  memset(&this->client, 0, sizeof(lwmqtt_client_t));

  // allocate buffers
  this->bufSize = (size_t)bufSize;
  this->readBuf = (uint8_t *)malloc((size_t)bufSize + 1);
  this->writeBuf = (uint8_t *)malloc((size_t)bufSize);
}

MQTTClient::~MQTTClient() {
  // free will
  this->clearWill();

  // free hostname
  if (this->hostname != nullptr) {
    free((void *)this->hostname);
  }

  // free buffers
  free(this->readBuf);
  free(this->writeBuf);
}

void MQTTClient::begin(const char _hostname[], int _port, Client &_client) {
  // set hostname and port
  this->setHost(_hostname, _port);

  // set client
  this->netClient = &_client;

  // initialize client
  lwmqtt_init(&this->client, this->writeBuf, this->bufSize, this->readBuf, this->bufSize);

  // set timers
  lwmqtt_set_timers(&this->client, &this->timer1, &this->timer2, lwmqtt_arduino_timer_set, lwmqtt_arduino_timer_get);

  // set network
  lwmqtt_set_network(&this->client, &this->network, lwmqtt_arduino_network_read, lwmqtt_arduino_network_write);

  // set callback
  lwmqtt_set_callback(&this->client, (void *)&this->callback, MQTTClientHandler);
}

void MQTTClient::onMessage(MQTTClientCallbackSimple cb) {
  // set callback
  this->callback.client = this;
  this->callback.simple = cb;
  this->callback.advanced = nullptr;
}

void MQTTClient::onMessageAdvanced(MQTTClientCallbackAdvanced cb) {
  // set callback
  this->callback.client = this;
  this->callback.simple = nullptr;
  this->callback.advanced = cb;
}

void MQTTClient::setClockSource(MQTTClientClockSource cb) {
  this->timer1.millis = cb;
  this->timer2.millis = cb;
}

void MQTTClient::setHost(const char _hostname[], int _port) {
  // free hostname if set
  if (this->hostname != nullptr) {
    free((void *)this->hostname);
  }

  // set hostname and port
  this->hostname = strdup(_hostname);
  this->port = _port;
}

void MQTTClient::setWill(const char topic[], const char payload[], bool retained, int qos) {
  // return if topic is missing
  if (topic == nullptr || strlen(topic) == 0) {
    return;
  }

  // clear existing will
  this->clearWill();

  // allocate will
  this->will = (lwmqtt_will_t *)malloc(sizeof(lwmqtt_will_t));
  memset(this->will, 0, sizeof(lwmqtt_will_t));

  // set topic
  this->will->topic = lwmqtt_string(strdup(topic));

  // set payload if available
  if (payload != nullptr && strlen(payload) > 0) {
    this->will->payload = lwmqtt_string(strdup(payload));
  }

  // set flags
  this->will->retained = retained;
  this->will->qos = (lwmqtt_qos_t)qos;
}

void MQTTClient::clearWill() {
  // return if not set
  if (this->will == nullptr) {
    return;
  }

  // free payload if set
  if (this->will->payload.len > 0) {
    free(this->will->payload.data);
  }

  // free topic if set
  if (this->will->topic.len > 0) {
    free(this->will->topic.data);
  }

  // free will
  free(this->will);
  this->will = nullptr;
}

void MQTTClient::setKeepAlive(int _keepAlive) { this->keepAlive = _keepAlive; }

void MQTTClient::setCleanSession(bool _cleanSession) { this->cleanSession = _cleanSession; }

void MQTTClient::setTimeout(int _timeout) { this->timeout = _timeout; }

void MQTTClient::setOptions(int _keepAlive, bool _cleanSession, int _timeout) {
  this->setKeepAlive(_keepAlive);
  this->setCleanSession(_cleanSession);
  this->setTimeout(_timeout);
}

bool MQTTClient::publish(const char topic[], const char payload[], int length, bool retained, int qos) {
  // return immediately if not connected
  if (!this->connected()) {
    return false;
  }

  // prepare message
  lwmqtt_message_t message = lwmqtt_default_message;
  message.payload = (uint8_t *)payload;
  message.payload_len = (size_t)length;
  message.retained = retained;
  message.qos = lwmqtt_qos_t(qos);

  // publish message
  this->_lastError = lwmqtt_publish(&this->client, lwmqtt_string(topic), message, this->timeout);
  if (this->_lastError != LWMQTT_SUCCESS) {
    // close connection
    this->close();

    return false;
  }

  return true;
}

bool MQTTClient::connect(const char clientId[], const char username[], const char password[], bool skip) {
  // close left open connection if still connected
  if (!skip && this->connected()) {
    this->close();
  }

  // save client
  this->network.client = this->netClient;

  // connect to host
  if (!skip) {
    int ret = this->netClient->connect(this->hostname, (uint16_t)this->port);
    if (ret <= 0) {
      this->_lastError = LWMQTT_NETWORK_FAILED_CONNECT;
      return false;
    }
  }

  // prepare options
  lwmqtt_options_t options = lwmqtt_default_options;
  options.keep_alive = this->keepAlive;
  options.clean_session = this->cleanSession;
  options.client_id = lwmqtt_string(clientId);

  // set username and password if available
  if (username != nullptr) {
    options.username = lwmqtt_string(username);

    if (password != nullptr) {
      options.password = lwmqtt_string(password);
    }
  }

  // connect to broker
  this->_lastError = lwmqtt_connect(&this->client, options, this->will, &this->_returnCode, this->timeout);
  if (this->_lastError != LWMQTT_SUCCESS) {
    // close connection
    this->close();

    return false;
  }

  // set flag
  this->_connected = true;

  return true;
}

bool MQTTClient::subscribe(const char topic[], int qos) {
  // return immediately if not connected
  if (!this->connected()) {
    return false;
  }

  // subscribe to topic
  this->_lastError = lwmqtt_subscribe_one(&this->client, lwmqtt_string(topic), (lwmqtt_qos_t)qos, this->timeout);
  if (this->_lastError != LWMQTT_SUCCESS) {
    // close connection
    this->close();

    return false;
  }

  return true;
}

bool MQTTClient::unsubscribe(const char topic[]) {
  // return immediately if not connected
  if (!this->connected()) {
    return false;
  }

  // unsubscribe from topic
  this->_lastError = lwmqtt_unsubscribe_one(&this->client, lwmqtt_string(topic), this->timeout);
  if (this->_lastError != LWMQTT_SUCCESS) {
    // close connection
    this->close();

    return false;
  }

  return true;
}

bool MQTTClient::loop() {
  // return immediately if not connected
  if (!this->connected()) {
    return false;
  }

  // get available bytes on the network
  auto available = (size_t)this->netClient->available();

  // yield if data is available
  if (available > 0) {
    this->_lastError = lwmqtt_yield(&this->client, available, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
      // close connection
      this->close();

      return false;
    }
  }

  // keep the connection alive
  this->_lastError = lwmqtt_keep_alive(&this->client, this->timeout);
  if (this->_lastError != LWMQTT_SUCCESS) {
    // close connection
    this->close();

    return false;
  }

  return true;
}

bool MQTTClient::connected() {
  // a client is connected if the network is connected, a client is available and
  // the connection has been properly initiated
  return this->netClient != nullptr && this->netClient->connected() == 1 && this->_connected;
}

bool MQTTClient::disconnect() {
  // return immediately if not connected anymore
  if (!this->connected()) {
    return false;
  }

  // cleanly disconnect
  this->_lastError = lwmqtt_disconnect(&this->client, this->timeout);

  // close
  this->close();

  return this->_lastError == LWMQTT_SUCCESS;
}

void MQTTClient::close() {
  // set flag
  this->_connected = false;

  // close network
  this->netClient->stop();
}
