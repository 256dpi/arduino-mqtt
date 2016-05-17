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

typedef struct {
    char * topic;
    char * payload;
    unsigned int length;
    boolean retained;
} MQTTMessage;

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
  boolean begin(const char * hostname, Client& client);
  boolean begin(const char * hostname, int port, Client& client);
  void setWill(const char * topic);
  void setWill(const char * topic, const char * payload);
  boolean connect(const char * clientId);
  boolean connect(const char * clientId, const char* username, const char* password);
  boolean publish(String topic);
  boolean publish(String topic, String payload);
  boolean publish(const char * topic, String payload);
  boolean publish(const char * topic, const char * payload);
  boolean publish(const char * topic, char * payload, unsigned int length);
  boolean publish(MQTTMessage * message);
  boolean subscribe(String topic);
  boolean subscribe(const char * topic);
  boolean unsubscribe(String topic);
  boolean unsubscribe(const char * topic);
  void loop();
  boolean connected();
  boolean disconnect();
};

#endif
