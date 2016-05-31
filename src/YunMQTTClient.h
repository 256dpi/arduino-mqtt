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
  const char * willTopic = "";
  const char * willPayload = "";
  const char * tlsCaCerts = "";
  boolean alive = false;
  boolean updateBridge();
public:
  YunMQTTClient();
  boolean begin(const char * hostname);
  boolean begin(const char * hostname, int port);
  void setWill(const char * topic);
  void setWill(const char * topic, const char * payload);
  void setTls(const char * caCerts);
  boolean connect(const char * clientId);
  boolean connect(const char * clientId, const char* username, const char* password);
  void publish(String topic);
  void publish(String topic, String payload);
  void publish(const char * topic, String payload);
  void publish(const char * topic, const char * payload);
  void publish(const char * topic, char * payload, unsigned int length);
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
