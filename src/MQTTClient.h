#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include <Stream.h>

#include "system.h"

typedef void (*MQTTClientCallback)(String topic, String payload, char bytes[], unsigned int length);

static void MQTTClient_callback(lwmqtt_client_t *client, void *ref, lwmqtt_string_t *topic, lwmqtt_message_t *message) {
  // get callback
  MQTTClientCallback cb = (MQTTClientCallback)ref;

  // null terminate topic to create String object
  char t[topic->len + 1];
  memcpy(topic, topic->data, (size_t)topic->len);
  t[topic->len] = '\0';

  // get payload
  char *payload = (char *)message->payload;

  // null terminate payload
  payload[message->payload_len] = '\0';

  // call the user callback
  cb(String(t), String(payload), (char *)message->payload, (unsigned int)message->payload_len);
}

template <int BUF_SIZE>
class AdvancedMQTTClient {
 private:
  unsigned char readBuf[BUF_SIZE + 1];  // plus one byte to ensure null termination
  unsigned char writeBuf[BUF_SIZE];

  unsigned int timeout = 1000;

  Client *netClient;
  const char *hostname;
  int port;
  lwmqtt_will_t will;
  bool hasWill;

  lwmqtt_arduino_network_t network;
  lwmqtt_arduino_timer_t timer1;
  lwmqtt_arduino_timer_t timer2;
  lwmqtt_client_t client;

  bool _connected = false;
  lwmqtt_return_code_t _returnCode;
  lwmqtt_err_t _lastError;

 public:
  AdvancedMQTTClient() {}

  void begin(Client &client) { this->begin("", client); }

  void begin(const char *hostname, Client &client) { this->begin(hostname, 1883, client); }

  void begin(const char *hostname, int port, Client &client) {
    // set config
    this->hostname = hostname;
    this->port = port;
    this->netClient = &client;

    // initialize client
    lwmqtt_init(&this->client, this->writeBuf, BUF_SIZE, this->readBuf, BUF_SIZE);

    // set timers
    lwmqtt_set_timers(&this->client, &this->timer1, &this->timer2, lwmqtt_arduino_timer_set, lwmqtt_arduino_timer_get);

    // set network
    lwmqtt_set_network(&this->client, &this->network, lwmqtt_arduino_network_read, lwmqtt_arduino_network_write);
  }

  void onMessage(MQTTClientCallback cb) {
    // unset callback if NULL is supplied
    if (cb == NULL) {
      lwmqtt_set_callback(&this->client, NULL, NULL);
      return;
    }

    // set callback
    lwmqtt_set_callback(&this->client, (void*)cb, MQTTClient_callback);
  }

  void setHost(const char *hostname) { this->setHost(hostname, 1883); }

  void setHost(const char *hostname, int port) {
    this->hostname = hostname;
    this->port = port;
  }

  void setWill(const char *topic) { this->setWill(topic, ""); }

  void setWill(const char *topic, const char *payload) { this->setWill(topic, payload, false, 0); }

  void setWill(const char *topic, const char *payload, bool retained, int qos) {
    this->hasWill = true;
    this->will.topic = lwmqtt_str(topic);
    this->will.payload = (void *)payload;
    this->will.payload_len = (int)strlen(payload);
    this->will.retained = retained;
    this->will.qos = (lwmqtt_qos_t)qos;
  }

  void clearWill() { this->hasWill = false; }

  boolean connect(const char *clientId) { return this->connect(clientId, NULL, NULL); }

  boolean connect(const char *clientId, const char *username, const char *password) {
    // TODO: What to do if already connected?

    // connect to network
    this->_lastError = lwmqtt_arduino_network_connect(&this->network, this->netClient, (char *)this->hostname, this->port);
    if (this->_lastError != LWMQTT_SUCCESS) {
      this->_connected = false;
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

  boolean publish(String topic) { return this->publish(topic.c_str(), ""); }

  boolean publish(String topic, String payload) { return this->publish(topic.c_str(), payload.c_str()); }

  boolean publish(const char *topic, String payload) { return this->publish(topic, payload.c_str()); }

  boolean publish(const char *topic, const char *payload) {
    return this->publish(topic, (char *)payload, (unsigned int)strlen(payload));
  }

  boolean publish(const char *topic, char *payload, unsigned int length) {
    return this->publish(topic, payload, length, false, 0);
  }

  boolean publish(const char *topic, char *payload, unsigned int length, bool retained, int qos) {
    // return immediately if not connected
    if (!this->_connected) {
      return false;
    }

    // prepare message
    lwmqtt_message_t message = lwmqtt_default_message;
    message.payload = payload;
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

  boolean subscribe(String topic) { return this->subscribe(topic.c_str()); }

  boolean subscribe(const char *topic) { this->subscribe(topic, 0); }

  boolean subscribe(const char *topic, int qos) {
    // return immediately if not connected
    if (!this->_connected) {
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

  boolean unsubscribe(String topic) { return this->unsubscribe(topic.c_str()); }

  boolean unsubscribe(const char *topic) {
    // return immediately if not connected
    if (!this->_connected) {
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
    if (!this->_connected) {
      return false;
    }

    // get available bytes
    int available = 0;
    lwmqtt_arduino_network_peek(&this->client, &this->network, &available);

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

  boolean connected() { return this->_connected; }

  lwmqtt_err_t lastError() { return this->_lastError; }

  lwmqtt_return_code_t returnCode() { return this->_returnCode; }

  boolean disconnect() {
    this->_connected = false;
    this->_lastError = lwmqtt_disconnect(&this->client, this->timeout);
    lwmqtt_arduino_network_disconnect(&this->network);
    return this->_lastError == LWMQTT_SUCCESS;
  }
};

class MQTTClient : public AdvancedMQTTClient<128> {};

#endif
