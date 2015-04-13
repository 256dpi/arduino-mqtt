#include "YunMQTTClient.h"

#include <FileIO.h>

YunMQTTClient::YunMQTTClient(const char * _hostname, int _port) {
  this->hostname = _hostname;
  this->port = _port;
}

boolean YunMQTTClient::installBridge(boolean force) {
  if(!force) {
    boolean f1 = FileSystem.exists("/usr/mqtt/mqtt.py");
    boolean f2 = FileSystem.exists("/usr/mqtt/bridge.py");

    if(f1 && f2) {
      return true;
    }
  }

  Process p;

  int r1 = p.runShellCommand("mkdir -p /usr/mqtt");
  // shortened: https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-bridge/yun/mqtt.py
  int r2 = p.runShellCommand("wget http://bit.ly/1aX4HVu --no-check-certificate -O /usr/mqtt/mqtt.py");
  // shortened: https://raw.githubusercontent.com/256dpi/arduino-mqtt/yun-bridge/yun/bridge.py
  int r3 = p.runShellCommand("wget http://bit.ly/1IXBeFe --no-check-certificate -O /usr/mqtt/bridge.py");

  return r1 == 0 && r2 == 0 && r3 == 0;
}

boolean YunMQTTClient::connect(const char * clientId) {
  return this->connect(clientId, "", "");
}

boolean YunMQTTClient::connect(const char * clientId, const char * username, const char * password) {
  this->process.begin("python");
  this->process.addParameter("-u");
  this->process.addParameter("/usr/mqtt/bridge.py");
  this->process.runAsynchronously();
  this->process.setTimeout(10000);

  // wait for script to launch
  this->process.readStringUntil('\n');

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
  this->alive = ret.equals("ca");

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
