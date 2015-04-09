#ifndef YUN_MQTT_CLIENT_H
#define YUN_MQTT_CLIENT_H

#include <Arduino.h>
#include <Client.h>

void messageReceived(String topic, String payload, char * bytes, unsigned int length);

class YunMQTTClient {
public:
  YunMQTTClient(const char * hostname, int port, Client& client);
  boolean connect(const char * clientId);
  boolean connect(const char * clientId, const char* username, const char* password);
  boolean publish(String topic);
  boolean publish(String topic, String payload);
  boolean publish(const char * topic, String payload);
  boolean publish(const char * topic, const char * payload);
  boolean subscribe(String topic);
  boolean subscribe(const char * topic);
  boolean unsubscribe(String topic);
  boolean unsubscribe(const char * topic);
  boolean loop();
  boolean connected();
  void disconnect();
};

#endif //YUN_MQTT_CLIENT_H
