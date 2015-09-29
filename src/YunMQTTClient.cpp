#ifdef ARDUINO_AVR_YUN

#include "YunMQTTClient.h"

#include <FileIO.h>

YunMQTTClient::YunMQTTClient() {}

void YunMQTTClient::begin(const char * hostname) {
  this->begin(hostname, 1883);
}

void YunMQTTClient::begin(const char * hostname, int port) {
  this->hostname = hostname;
  this->port = port;
}

int YunMQTTClient::updateBridge() {
  Process p;

  int r1 = p.runShellCommand("mkdir -p /usr/arduino-mqtt");
  int r2 = p.runShellCommand("wget -N https://raw.githubusercontent.com/256dpi/arduino-mqtt/v1.8.0/yun/mqtt.py --no-check-certificate -P /usr/arduino-mqtt");
  int r3 = p.runShellCommand("wget -N https://raw.githubusercontent.com/256dpi/arduino-mqtt/v1.8.0/yun/bridge.py --no-check-certificate -P /usr/arduino-mqtt");

  boolean success = r1 == 0 && r2 == 0 && r3 == 0;

  if(success) {
    return 2;
  } else {
    return 0;
  }
}

void YunMQTTClient::setWill(const char * topic) {
  this->setWill(topic, "");
}

void YunMQTTClient::setWill(const char * topic, const char * payload) {
  this->willTopic = topic;
  this->willPayload = payload;
}

boolean YunMQTTClient::connect(const char * clientId) {
  return this->connect(clientId, "", "");
}

boolean YunMQTTClient::connect(const char * clientId, const char * username, const char * password) {
  if(this->updateBridge() == 0) {
    return false;
  }

  this->process.begin("python");
  this->process.addParameter("-u");
  this->process.addParameter("/usr/mqtt/bridge.py");
  this->process.runAsynchronously();
  this->process.setTimeout(10000);

  // wait for script to launch
  this->process.readStringUntil('\n');

  // set will if available
  if(this->willTopic != NULL) {
    this->process.print("w:");
    this->process.print(this->willTopic);
    if(this->willPayload != NULL) {
      this->process.print(':');
      this->process.print(this->willPayload);
    }
    this->process.print('\n');
  }

  // send connect request
  this->process.print("c:");
  this->process.print(this->hostname);
  this->process.print(':');
  this->process.print(this->port);
  this->process.print(':');
  this->process.print(clientId);
  if(strlen(username) > 0 && strlen(password) > 0) {
    this->process.print(':');
    this->process.print(username);
    this->process.print(':');
    this->process.print(password);
  }
  this->process.print('\n');

  // wait for answer
  String ret = this->process.readStringUntil('\n');
  this->alive = ret.equals("a");

  if(!this->alive) {
    this->process.close();
    return false;
  } else {
    return true;
  }
}

void YunMQTTClient::publish(String topic) {
  this->publish(topic.c_str(), "");
}

void YunMQTTClient::publish(String topic, String payload) {
  this->publish(topic.c_str(), payload.c_str());
}

void YunMQTTClient::publish(const char * topic, String payload) {
  this->publish(topic, payload.c_str());
}

void YunMQTTClient::publish(const char * topic, const char * payload) {
  // send publish request
  this->process.print("p:");
  this->process.print(topic);
  this->process.print(':');
  this->process.print(payload);
  this->process.print('\n');
}

void YunMQTTClient::subscribe(String topic) {
  this->subscribe(topic.c_str());
}

void YunMQTTClient::subscribe(const char * topic) {
  // send subscribe request
  this->process.print("s:");
  this->process.print(topic);
  this->process.print('\n');
}

void YunMQTTClient::unsubscribe(String topic) {
  this->unsubscribe(topic.c_str());
}

void YunMQTTClient::unsubscribe(const char * topic) {
  // send unsubscribe request
  this->process.print("u:");
  this->process.print(topic);
  this->process.print('\n');
}

void YunMQTTClient::loop() {
  int av = this->process.available();
  if(av > 0) {
    String ret = process.readStringUntil('\n');

    if(ret.startsWith("m")) {
      int startTopic = 2;
      int endTopic = ret.indexOf(':', startTopic + 1);
      int startPayload = endTopic + 1;
      int endPayload = ret.indexOf(':', startPayload + 1);
      String topic = ret.substring(startTopic, endTopic);
      String payload = ret.substring(startPayload, endPayload);
      messageReceived(topic, payload, (char*)payload.c_str(), payload.length());
    } else if(ret.startsWith("e")) {
      this->alive = false;
    }
  }
}

boolean YunMQTTClient::connected() {
  return this->alive;
}

void YunMQTTClient::disconnect() {
  // send disconnect request
  this->process.print("d\n");
}

#endif //ARDUINO_AVR_YUN
