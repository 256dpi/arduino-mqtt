#ifndef YUN_MQTT_CLIENT_H
#define YUN_MQTT_CLIENT_H

#include <Arduino.h>
#include <Brdige.h>
#include <Process.h>

void messageReceived(String topic, String payload, char * bytes, unsigned int length);

class YunMQTTClient {
public:
  YunMQTTClient(const char * hostname, int port);
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

#endif //YUN_MQTT_CLIENT_H
