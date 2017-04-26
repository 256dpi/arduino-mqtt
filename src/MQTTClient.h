#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include <Stream.h>

#include "system.h"

typedef struct {
  char *topic;
  char *payload;
  unsigned int length;
  boolean retained;
} MQTTMessage;

void messageReceived(String topic, String payload, char *bytes, unsigned int length);

void MQTTClient_callback(lwmqtt_client_t *client, lwmqtt_string_t *topic, lwmqtt_message_t *message) {
  // null terminate topic to create String object
  char t[topic->len + 1];
  memcpy(topic, topic->data, (size_t)topic->len);
  t[topic->len] = '\0';

  // get payload
  char *payload = (char *)message->payload;

  // TODO: Only do this if enough space is available...
  // null terminate payload
  payload[message->payload_len] = '\0';

  messageReceived(String(t), String(payload), (char *)message->payload, (unsigned int)message->payload_len);
}

template <int BUF_SIZE>
class AdvancedMQTTClient {
 private:
  unsigned char readBuf[BUF_SIZE];
  unsigned char writeBuf[BUF_SIZE];

  unsigned int timeout = 1000;

  const char *hostname;
  int port;
  lwmqtt_will_t will;
  bool hasWill;

  bool isConnected = false;
  Client *netClient;
  lwmqtt_err_t err;

  lwmqtt_arduino_network_t network;
  lwmqtt_arduino_timer_t timer1;
  lwmqtt_arduino_timer_t timer2;
  lwmqtt_client_t client;

 public:
  AdvancedMQTTClient() {}

  boolean begin(Client &client) { return this->begin("", client); }

  boolean begin(const char *hostname, Client &client) { return this->begin(hostname, 1883, client); }

  boolean begin(const char *hostname, int port, Client &client) {
    // set config
    this->hostname = hostname;
    this->port = port;

    // initialize client
    lwmqtt_init(&this->client, this->writeBuf, BUF_SIZE, this->readBuf, BUF_SIZE);

    // set timers
    lwmqtt_set_timers(&this->client, &this->timer1, &this->timer2, lwmqtt_arduino_timer_set, lwmqtt_arduino_timer_get);

    // set network
    lwmqtt_set_network(&this->client, &this->network, lwmqtt_arduino_network_read, lwmqtt_arduino_network_write);

    // save net client
    this->netClient = &client;

    // set callback
    lwmqtt_set_callback(&this->client, MQTTClient_callback);

    return true;
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
    // connect to network
    this->err = lwmqtt_arduino_network_connect(&this->network, this->netClient, (char *)this->hostname, this->port);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
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
    lwmqtt_return_code_t returnCode;
    this->err = lwmqtt_connect(&this->client, &options, will, &returnCode, this->timeout);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
      return false;
    }

    // set flag
    this->isConnected = true;

    return true;
  }

  boolean publish(String topic) { return this->publish(topic.c_str(), ""); }

  boolean publish(String topic, String payload) { return this->publish(topic.c_str(), payload.c_str()); }

  boolean publish(const char *topic, String payload) { return this->publish(topic, payload.c_str()); }

  boolean publish(const char *topic, const char *payload) {
    return this->publish(topic, (char *)payload, (unsigned int)strlen(payload));
  }

  boolean publish(const char *topic, char *payload, unsigned int length) {
    // prepare message
    lwmqtt_message_t message = lwmqtt_default_message;
    message.qos = LWMQTT_QOS0;
    message.retained = false;
    message.payload = payload;
    message.payload_len = length;

    // publish message
    this->err = lwmqtt_publish(&this->client, topic, &message, this->timeout);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
      return false;
    }

    return true;
  }

  boolean publish(MQTTMessage *message) {
    // prepare message
    lwmqtt_message_t _message = lwmqtt_default_message;
    _message.qos = LWMQTT_QOS0;
    _message.retained = message->retained;
    _message.payload = message->payload;
    _message.payload_len = message->length;

    // publish message
    this->err = lwmqtt_publish(&this->client, message->topic, &_message, this->timeout);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
      return false;
    }

    return true;
  }

  // TODO: Add QOS.

  boolean subscribe(String topic) { return this->subscribe(topic.c_str()); }

  boolean subscribe(const char *topic) { this->subscribe(topic, 0); }

  boolean subscribe(const char *topic, int qos) {
    this->err = lwmqtt_subscribe(&this->client, topic, (lwmqtt_qos_t)qos, this->timeout);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
      return false;
    }

    return true;
  }

  boolean unsubscribe(String topic) { return this->unsubscribe(topic.c_str()); }

  boolean unsubscribe(const char *topic) {
    this->err = lwmqtt_unsubscribe(&this->client, topic, this->timeout);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
      return false;
    }

    return true;
  }

  boolean loop() {
    // return immediately if not connected
    if (!this->isConnected) {
      return false;
    }

    // get available bytes
    int available = 0;
    lwmqtt_arduino_network_peek(&this->client, &this->network, &available);

    // yield if data is available
    if (available > 0) {
      this->err = lwmqtt_yield(&this->client, available, this->timeout);
      if (this->err != LWMQTT_SUCCESS) {
        this->isConnected = false;
        return false;
      }
    }

    // keep the connection alive
    this->err = lwmqtt_keep_alive(&this->client, this->timeout);
    if (this->err != LWMQTT_SUCCESS) {
      this->isConnected = false;
      return false;
    }

    return true;
  }

  boolean connected() { return this->isConnected; }

  boolean disconnect() {
    this->isConnected = false;
    this->err = lwmqtt_disconnect(&this->client, this->timeout);
    lwmqtt_arduino_network_disconnect(&this->network);
    return this->err == LWMQTT_SUCCESS;
  }
};

class MQTTClient : public AdvancedMQTTClient<128> {};

#endif
