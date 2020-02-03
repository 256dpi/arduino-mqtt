#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <Client.h>
#include <Stream.h>

extern "C" {
#include "lwmqtt/lwmqtt.h"
}

typedef uint32_t (*MQTTClientClockSource)();

typedef struct {
  uint32_t end;
  MQTTClientClockSource millis;
} lwmqtt_arduino_timer_t;

typedef struct {
  Client *client;
} lwmqtt_arduino_network_t;

class MQTTClient;

typedef void (*MQTTClientCallbackSimple)(String &topic, String &payload);
typedef void (*MQTTClientCallbackAdvanced)(MQTTClient *client, char topic[], char bytes[], int length);

typedef struct {
  MQTTClient *client = nullptr;
  MQTTClientCallbackSimple simple = nullptr;
  MQTTClientCallbackAdvanced advanced = nullptr;
} MQTTClientCallback;

class MQTTClient {
 private:
  size_t bufSize = 0;
  uint8_t *readBuf = nullptr;
  uint8_t *writeBuf = nullptr;

  uint16_t keepAlive = 10;
  bool cleanSession = true;
  uint32_t timeout = 1000;

  Client *netClient = nullptr;
  const char *hostname = nullptr;
  int port = 0;
  lwmqtt_will_t *will = nullptr;
  MQTTClientCallback callback;

  lwmqtt_arduino_network_t network = {nullptr};
  lwmqtt_arduino_timer_t timer1 = {0, nullptr};
  lwmqtt_arduino_timer_t timer2 = {0, nullptr};
  lwmqtt_client_t client = {0};

  bool _connected = false;
  lwmqtt_return_code_t _returnCode = (lwmqtt_return_code_t)0;
  lwmqtt_err_t _lastError = (lwmqtt_err_t)0;

 public:
  void *ref = nullptr;

  explicit MQTTClient(int bufSize = 128);

  ~MQTTClient();

  void begin(const char _hostname[], Client &_client) { this->begin(_hostname, 1883, _client); }
  void begin(const char hostname[], int port, Client &client);

  void onMessage(MQTTClientCallbackSimple cb);
  void onMessageAdvanced(MQTTClientCallbackAdvanced cb);

  void setClockSource(MQTTClientClockSource cb);

  void setHost(const char _hostname[]) { this->setHost(_hostname, 1883); }
  void setHost(const char hostname[], int port);

  void setWill(const char topic[]) { this->setWill(topic, ""); }
  void setWill(const char topic[], const char payload[]) { this->setWill(topic, payload, false, 0); }
  void setWill(const char topic[], const char payload[], bool retained, int qos);
  void clearWill();

  void setKeepAlive(int keepAlive);
  void setCleanSession(bool cleanSession);
  void setTimeout(int timeout);
  void setOptions(int keepAlive, bool cleanSession, int timeout);

  bool connect(const char clientId[], bool skip = false) { return this->connect(clientId, nullptr, nullptr, skip); }
  bool connect(const char clientId[], const char username[], bool skip = false) {
    return this->connect(clientId, username, nullptr, skip);
  }
  bool connect(const char clientId[], const char username[], const char password[], bool skip = false);

  bool publish(const String &topic) { return this->publish(topic.c_str(), ""); }
  bool publish(const char topic[]) { return this->publish(topic, ""); }
  bool publish(const String &topic, const String &payload) { return this->publish(topic.c_str(), payload.c_str()); }
  bool publish(const String &topic, const String &payload, bool retained, int qos) {
    return this->publish(topic.c_str(), payload.c_str(), retained, qos);
  }
  bool publish(const char topic[], const String &payload) { return this->publish(topic, payload.c_str()); }
  bool publish(const char topic[], const String &payload, bool retained, int qos) {
    return this->publish(topic, payload.c_str(), retained, qos);
  }
  bool publish(const char topic[], const char payload[]) {
    return this->publish(topic, (char *)payload, (int)strlen(payload));
  }
  bool publish(const char topic[], const char payload[], bool retained, int qos) {
    return this->publish(topic, (char *)payload, (int)strlen(payload), retained, qos);
  }
  bool publish(const char topic[], const char payload[], int length) {
    return this->publish(topic, payload, length, false, 0);
  }
  bool publish(const char topic[], const char payload[], int length, bool retained, int qos);

  bool subscribe(const String &topic) { return this->subscribe(topic.c_str()); }
  bool subscribe(const String &topic, int qos) { return this->subscribe(topic.c_str(), qos); }
  bool subscribe(const char topic[]) { return this->subscribe(topic, 0); }
  bool subscribe(const char topic[], int qos);

  bool unsubscribe(const String &topic) { return this->unsubscribe(topic.c_str()); }
  bool unsubscribe(const char topic[]);

  bool loop();
  bool connected();

  lwmqtt_err_t lastError() { return this->_lastError; }
  lwmqtt_return_code_t returnCode() { return this->_returnCode; }

  bool disconnect();

 private:
  void close();
};

#endif
