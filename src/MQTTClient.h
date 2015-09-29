#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 128
#endif

#define MQTTCLIENT_QOS1 0
#define MQTTCLIENT_QOS2 0

#include <Arduino.h>
#include <Client.h>
#include <Stream.h>
#include "lib/MQTTClient.h"
#include "Network.h"
#include "Timer.h"

void messageReceived(String topic, String payload, char * bytes, unsigned int length);

class MQTTClient {
private:
  Network network;
  MQTT::Client<Network, Timer, MQTT_BUFFER_SIZE, 0> * client;
  MQTTPacket_connectData options;
  const char * hostname;
  int port;
public:
  MQTTClient();
  void begin(const char * hostname, Client& client);
  void begin(const char * hostname, int port, Client& client);
  void setWill(const char * topic);
  void setWill(const char * topic, const char * payload);
  boolean connect(const char * clientId);
  boolean connect(const char * clientId, const char* username, const char* password);
  void publish(String topic);
  void publish(String topic, String payload);
  void publish(const char * topic, String payload);
  void publish(const char * topic, const char * payload);
  void subscribe(String topic);
  void subscribe(const char * topic);
  void unsubscribe(String topic);
  void unsubscribe(const char * topic);
  void loop();
  boolean connected();
  void disconnect();
};

#endif
