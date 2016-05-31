#include "YunMQTTClient.h"

#ifdef YUN_MQTT_CLIENT_ENABLED

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

  int r1 = p.runShellCommand(F("mkdir -p /usr/arduino-mqtt"));
  int r2 = p.runShellCommand(F("wget -N https://raw.githubusercontent.com/256dpi/arduino-mqtt/v1.10.1/yun/mqtt.py --no-check-certificate -P /usr/arduino-mqtt"));
  int r3 = p.runShellCommand(F("wget -N https://raw.githubusercontent.com/256dpi/arduino-mqtt/v1.10.1/yun/bridge.py --no-check-certificate -P /usr/arduino-mqtt"));

  return r1 == 0 && r2 == 0 && r3 == 0;
}

void YunMQTTClient::setWill(const char * topic) {
  this->setWill(topic, "");
}

void YunMQTTClient::setWill(const char * topic, const char * payload) {
  this->willTopic = topic;
  this->willPayload = payload;
}

void YunMQTTClient::setTls(const char * caCerts) {
  this->tlsCaCerts = caCerts;
}

boolean YunMQTTClient::connect(const char * clientId) {
  return this->connect(clientId, "", "");
}

boolean YunMQTTClient::connect(const char * clientId, const char * username, const char * password) {
  this->process.begin(F("python"));
  this->process.addParameter(F("-u"));
  this->process.addParameter(F("/usr/arduino-mqtt/bridge.py"));
  this->process.runAsynchronously();
  this->process.setTimeout(10000);

  // wait for script to launch
  this->process.readStringUntil('\n');

  // set will if available
  if(strlen(this->willTopic) > 0) {
    this->process.print(F("w:"));
    this->process.print(this->willTopic);
    this->process.print(F(":"));
    this->process.print(strlen(this->willPayload));
    this->process.print(F(";"));
    this->process.print(this->willPayload);
    this->process.print(F("\n"));
  }

  // set TLS if available
  if(strlen(this->tlsCaCerts) > 0) {
    this->process.print("t:");
    this->process.print(this->tlsCaCerts);
    this->process.print(";\n");
  }

  // send connect request
  this->process.print(F("c:"));
  this->process.print(this->hostname);
  this->process.print(F(":"));
  this->process.print(this->port);
  this->process.print(F(":"));
  this->process.print(clientId);
  if(strlen(username) > 0 && strlen(password) > 0) {
    this->process.print(F(":"));
    this->process.print(username);
    this->process.print(F(":"));
    this->process.print(password);
  }
  this->process.print(F(";\n"));

  // wait for answer
  String ret = this->process.readStringUntil('\n');
  this->alive = ret.equals(F("a;"));

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
  this->process.print(F("p:"));
  this->process.print(topic);
  this->process.print(F(":"));
  this->process.print(length);
  this->process.print(F(";"));

  for(int i=0; i<length; i++) {
    this->process.write(payload[i]);
  }

  this->process.print(F("\n"));
}

void YunMQTTClient::subscribe(String topic) {
  this->subscribe(topic.c_str());
}

void YunMQTTClient::subscribe(const char * topic) {
  // send subscribe request
  this->process.print(F("s:"));
  this->process.print(topic);
  this->process.print(F(";\n"));
}

void YunMQTTClient::unsubscribe(String topic) {
  this->unsubscribe(topic.c_str());
}

void YunMQTTClient::unsubscribe(const char * topic) {
  // send unsubscribe request
  this->process.print(F("u:"));
  this->process.print(topic);
  this->process.print(F(";\n"));
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
  this->process.print(F("d;\n"));
}

#endif //YUN_MQTT_CLIENT_ENABLED
