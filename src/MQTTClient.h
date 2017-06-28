#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include <Stream.h>

#include "system.h"

class MQTTClient;

typedef void (*MQTTClientCallbackSimple)(String &topic, String &payload);
typedef void (*MQTTClientCallbackAdvanced)(MQTTClient * client, char bytes[], unsigned int length);

typedef struct {
  MQTTClient * client = NULL;
  bool use_advanced = false;
  MQTTClientCallbackSimple simple = NULL;
  MQTTClientCallbackAdvanced advanced = NULL;
} MQTTClientCallback;

static void MQTTClient_callback(lwmqtt_client_t *client, void *ref, lwmqtt_string_t *topic, lwmqtt_message_t *message) {
  // get callback
  MQTTClientCallback *cb = (MQTTClientCallback *)ref;

  // null terminate topic to create String object
  char terminated_topic[topic->len + 1];
  memcpy(terminated_topic, topic->data, (size_t)topic->len);
  terminated_topic[topic->len] = '\0';

  // get payload
  char *payload = (char *)message->payload;

  // null terminate payload
  payload[message->payload_len] = '\0';

  // call the user callback
  if (cb->use_advanced) {
    // call advanced callback
    cb->advanced(cb->client, (char *)message->payload, (unsigned int)message->payload_len);
  } else {
    // create arduino strings
    // TODO: Is there no way to create the strings without causing the data to be copied?
    String str_topic = String(terminated_topic);
    String str_payload = String(payload);

    // call simple callback
    cb->simple(str_topic, str_payload);
  }
}

class MQTTClient {
 private:
  int bufSize;
  unsigned char *readBuf;
  unsigned char *writeBuf;

  unsigned int timeout = 1000;

  Client *netClient;
  const char *hostname;
  int port;
  lwmqtt_will_t will;
  bool hasWill;
  MQTTClientCallback callback;

  lwmqtt_arduino_network_t network;
  lwmqtt_arduino_timer_t timer1;
  lwmqtt_arduino_timer_t timer2;
  lwmqtt_client_t client;

  bool _connected = false;
  lwmqtt_return_code_t _returnCode;
  lwmqtt_err_t _lastError;

 public:
  MQTTClient(int bufSize = 128) {
    this->bufSize = bufSize;
    this->readBuf = (unsigned char *)malloc((size_t)bufSize + 1);
    this->writeBuf = (unsigned char *)malloc((size_t)bufSize);
  }

  ~MQTTClient() {
    free(this->readBuf);
    free(this->writeBuf);
  }

  void begin(const char hostname[], Client &client) { this->begin(hostname, 1883, client); }

  void begin(const char hostname[], int port, Client &client) {
    // set config
    this->hostname = hostname;
    this->port = port;
    this->netClient = &client;

    // initialize client
    lwmqtt_init(&this->client, this->writeBuf, this->bufSize, this->readBuf, this->bufSize);

    // set timers
    lwmqtt_set_timers(&this->client, &this->timer1, &this->timer2, lwmqtt_arduino_timer_set, lwmqtt_arduino_timer_get);

    // set network
    lwmqtt_set_network(&this->client, &this->network, lwmqtt_arduino_network_read, lwmqtt_arduino_network_write);
  }

  void onMessage(MQTTClientCallbackSimple cb) {
    // unset callback if NULL is supplied
    if (cb == NULL) {
      lwmqtt_set_callback(&this->client, NULL, NULL);
      return;
    }

    // save callback
    this->callback.client = this;
    this->callback.use_advanced = false;
    this->callback.simple = cb;

    // set callback
    lwmqtt_set_callback(&this->client, (void *)&this->callback, MQTTClient_callback);
  }

  void onMessageAdvanced(MQTTClientCallbackAdvanced cb) {
    // unset callback if NULL is supplied
    if (cb == NULL) {
      lwmqtt_set_callback(&this->client, NULL, NULL);
      return;
    }

    // save callback
    this->callback.client = this;
    this->callback.use_advanced = true;
    this->callback.advanced = cb;

    // set callback
    lwmqtt_set_callback(&this->client, (void *)&this->callback, MQTTClient_callback);
  }

  void setHost(const char hostname[]) { this->setHost(hostname, 1883); }

  void setHost(const char hostname[], int port) {
    this->hostname = hostname;
    this->port = port;
  }

  void setWill(const char topic[]) { this->setWill(topic, ""); }

  void setWill(const char topic[], const char payload[]) { this->setWill(topic, payload, false, 0); }

  void setWill(const char topic[], const char payload[], bool retained, int qos) {
    this->hasWill = true;
    this->will.topic = lwmqtt_str(topic);
    this->will.message.payload = (void *)payload;
    this->will.message.payload_len = (int)strlen(payload);
    this->will.message.retained = retained;
    this->will.message.qos = (lwmqtt_qos_t)qos;
  }

  void clearWill() { this->hasWill = false; }

  boolean connect(const char clientId[]) { return this->connect(clientId, NULL, NULL); }

  boolean connect(const char clientId[], const char username[], const char password[]) {
    // return immediately if connected
    if (this->connected()) {
      return false;
    }

    // save client
    this->network.client = this->netClient;

    // connect to host
    if (this->netClient->connect(this->hostname, (uint16_t)this->port) < 0) {
      return false;
    }

    // prepare options
    lwmqtt_options_t options = lwmqtt_default_options;
    options.client_id = lwmqtt_str(clientId);
    if (username && password) {
      options.username = lwmqtt_str(username);
      options.password = lwmqtt_str(password);
    }

    // prepare will reference
    lwmqtt_will_t *will = NULL;
    if (this->hasWill) {
      will = &this->will;
    }

    // connect to broker
    this->_lastError = lwmqtt_connect(&this->client, &options, will, &this->_returnCode, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
      this->_connected = false;
      return false;
    }

    // set flag
    this->_connected = true;

    return true;
  }

  boolean publish(const String &topic) { return this->publish(topic.c_str(), ""); }

  boolean publish(const char topic[]) { return this->publish(topic, ""); }

  boolean publish(const String &topic, const String &payload) { return this->publish(topic.c_str(), payload.c_str()); }

  boolean publish(const String &topic, const String &payload, bool retained, int qos) {
    return this->publish(topic.c_str(), payload.c_str(), retained, qos);
  }

  boolean publish(const char topic[], const String &payload) { return this->publish(topic, payload.c_str()); }

  boolean publish(const char topic[], const String &payload, bool retained, int qos) {
    return this->publish(topic, payload.c_str(), retained, qos);
  }

  boolean publish(const char topic[], const char payload[]) {
    return this->publish(topic, (char *)payload, (unsigned int)strlen(payload));
  }

  boolean publish(const char topic[], const char payload[], bool retained, int qos) {
    return this->publish(topic, (char *)payload, (unsigned int)strlen(payload), retained, qos);
  }

  boolean publish(const char topic[], const char payload[], unsigned int length) {
    return this->publish(topic, payload, length, false, 0);
  }

  boolean publish(const char topic[], const char payload[], unsigned int length, bool retained, int qos) {
    // return immediately if not connected
    if (!this->connected()) {
      return false;
    }

    // prepare message
    lwmqtt_message_t message = lwmqtt_default_message;
    message.payload = (void*)payload;
    message.payload_len = length;
    message.retained = retained;
    message.qos = lwmqtt_qos_t(qos);

    // publish message
    this->_lastError = lwmqtt_publish(&this->client, topic, &message, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
      this->_connected = false;
      return false;
    }

    return true;
  }

  boolean subscribe(const String &topic) { return this->subscribe(topic.c_str()); }

  boolean subscribe(const String &topic, int qos) { return this->subscribe(topic.c_str(), qos); }

  boolean subscribe(const char topic[]) { this->subscribe(topic, 0); }

  boolean subscribe(const char topic[], int qos) {
    // return immediately if not connected
    if (!this->connected()) {
      return false;
    }

    // subscribe to topic
    this->_lastError = lwmqtt_subscribe(&this->client, topic, (lwmqtt_qos_t)qos, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
      this->_connected = false;
      return false;
    }

    return true;
  }

  boolean unsubscribe(const String &topic) { return this->unsubscribe(topic.c_str()); }

  boolean unsubscribe(const char topic[]) {
    // return immediately if not connected
    if (!this->connected()) {
      return false;
    }

    // unsubscribe from topic
    this->_lastError = lwmqtt_unsubscribe(&this->client, topic, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
      this->_connected = false;
      return false;
    }

    return true;
  }

  boolean loop() {
    // return immediately if not connected
    if (!this->connected()) {
      return false;
    }

    // get available bytes on the network
    int available = this->netClient->available();

    // yield if data is available
    if (available > 0) {
      this->_lastError = lwmqtt_yield(&this->client, available, this->timeout);
      if (this->_lastError != LWMQTT_SUCCESS) {
        this->_connected = false;
        return false;
      }
    }

    // keep the connection alive
    this->_lastError = lwmqtt_keep_alive(&this->client, this->timeout);
    if (this->_lastError != LWMQTT_SUCCESS) {
      this->_connected = false;
      return false;
    }

    return true;
  }

  boolean connected() {
    // a client is connected if the network is connected and
    // a connection has been properly initiated
    return this->netClient->connected() && this->_connected;
  }

  lwmqtt_err_t lastError() { return this->_lastError; }

  lwmqtt_return_code_t returnCode() { return this->_returnCode; }

  boolean disconnect() {
    this->_connected = false;
    this->_lastError = lwmqtt_disconnect(&this->client, this->timeout);
    this->netClient->stop();
    return this->_lastError == LWMQTT_SUCCESS;
  }
};

#endif
