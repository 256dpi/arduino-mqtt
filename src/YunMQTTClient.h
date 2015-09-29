#ifndef YUN_MQTT_CLIENT_H
#define YUN_MQTT_CLIENT_H

#ifdef ARDUINO_AVR_YUN

#include <Arduino.h>
#include <Bridge.h>

void messageReceived(String topic, String payload, char * bytes, unsigned int length);

class YunMQTTClient {
private:
  Process process;
  const char * hostname;
  int port;
  const char * willTopic = NULL;
  const char * willPayload = NULL;
  boolean alive = false;
  int updateBridge();
public:
  YunMQTTClient();
  void begin(const char * hostname);
  void begin(const char * hostname, int port);
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

#endif //ARDUINO_AVR_YUN
#endif //YUN_MQTT_CLIENT_H
