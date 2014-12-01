#ifndef MQTTClient_h
#define MQTTClient_h

#define MQTTCLIENT_QOS1 0
#define MQTTCLIENT_QOS2 0

#include <Arduino.h>
#include <Client.h>
#include <Stream.h>
#include <lib/MQTTClient.h>
#include "Network.h"
#include "Timer.h"

void messageReceived(String topic, String payload);

class MQTTClient {
private:
  Network network;
  MQTT::Client<Network, Timer> * client;
  const char * hostname;
  int port;
public:
  MQTTClient(const char * hostname, int port, Client& client);
  boolean connect(const char * clientId);
  boolean connect(const char * clientId, const char* username, const char* password);
  boolean publish(const char * topic, String payload);
  boolean publish(const char * topic, const char * payload);
  boolean subscribe(const char * topic);
  boolean unsubscribe(const char * topic);
  boolean loop();
  boolean connected();
  void disconnect();
};

#endif
