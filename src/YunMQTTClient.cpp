#ifdef ARDUINO_AVR_YUN

#include "YunMQTTClient.h"

#include <FileIO.h>

YunMQTTClient::YunMQTTClient() {}

boolean YunMQTTClient::begin(const char * hostname) {
  return this->begin(hostname, 1883);
}

boolean YunMQTTClient::begin(const char * hostname, int port) {
  this->hostname = hostname;
  this->port = port;

  return this->updateBridge();
}

boolean YunMQTTClient::updateBridge() {
  Process p;

  int r1 = p.runShellCommand("mkdir -p /usr/arduino-mqtt");
  int r2 = p.runShellCommand("wget -N https://raw.githubusercontent.com/256dpi/arduino-mqtt/v1.10.0/yun/mqtt.py --no-check-certificate -P /usr/arduino-mqtt");
  int r3 = p.runShellCommand("wget -N https://raw.githubusercontent.com/256dpi/arduino-mqtt/v1.10.0/yun/bridge.py --no-check-certificate -P /usr/arduino-mqtt");

  return r1 == 0 && r2 == 0 && r3 == 0;
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
  this->process.begin("python");
  this->process.addParameter("-u");
  this->process.addParameter("/usr/arduino-mqtt/bridge.py");
  this->process.runAsynchronously();
  this->process.setTimeout(10000);

  // wait for script to launch
  this->process.readStringUntil('\n');

  // set will if available
  if(strlen(this->willTopic) > 0) {
    this->process.print("w:");
    this->process.print(this->willTopic);
    this->process.print(':');
    this->process.print(strlen(this->willPayload));
    this->process.print(';');
    this->process.print(this->willPayload);
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
  this->process.print(";\n");

  // wait for answer
  String ret = this->process.readStringUntil('\n');
  this->alive = ret.equals("a;");

  if(!this->alive) {
    this->process.close();
    return false;
  }

  return true;
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
  this->publish(topic, (char*)payload, strlen(payload));
}

void YunMQTTClient::publish(const char * topic, char * payload, unsigned int length) {
  // send publish request
  this->process.print("p:");
  this->process.print(topic);
  this->process.print(':');
  this->process.print(length);
  this->process.print(';');

  for(int i=0; i<length; i++) {
    this->process.write(payload[i]);
  }

  this->process.print('\n');
}

void YunMQTTClient::subscribe(String topic) {
  this->subscribe(topic.c_str());
}

void YunMQTTClient::subscribe(const char * topic) {
  // send subscribe request
  this->process.print("s:");
  this->process.print(topic);
  this->process.print(";\n");
}

void YunMQTTClient::unsubscribe(String topic) {
  this->unsubscribe(topic.c_str());
}

void YunMQTTClient::unsubscribe(const char * topic) {
  // send unsubscribe request
  this->process.print("u:");
  this->process.print(topic);
  this->process.print(";\n");
}

void YunMQTTClient::loop() {
  int av = this->process.available();
  if(av > 0) {
    String ret = process.readStringUntil(';');

    if(ret.startsWith("m")) {
      int startTopic = 2;
      int endTopic = ret.indexOf(':', startTopic + 1);
      String topic = ret.substring(startTopic, endTopic);

      int startPayloadLength = endTopic + 1;
      int endPayloadLength = ret.indexOf(':', startPayloadLength + 1);
      int payloadLength = ret.substring(startPayloadLength, endPayloadLength).toInt();

      char buf[payloadLength+1];
      process.readBytes(buf, payloadLength);
      buf[payloadLength] = '\0';

      messageReceived(topic, String(buf), buf, payloadLength);
    } else if(ret.startsWith("e")) {
      this->alive = false;
      this->process.close();
    }

    process.readStringUntil('\n');
  }
}

boolean YunMQTTClient::connected() {
  return this->process.running() && this->alive;
}

void YunMQTTClient::disconnect() {
  // send disconnect request
  this->process.print("d;\n");
}

#endif //ARDUINO_AVR_YUN
