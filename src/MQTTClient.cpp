#include "MQTTClient.h"


MQTTClient::MQTTClient() {}

boolean MQTTClient::begin(const char * hostname, Client& client) {
  return this->begin(hostname, 1883, client);
}

boolean MQTTClient::begin(const char * _hostname, int _port, Client& _client) {
  this->client = new MQTT::Client<MQTTClient, Network, Timer, MQTT_BUFFER_SIZE, 0>(this->network);
  this->network.setClient(&_client);
  this->client->defaultMessageHandler.attach<MQTTClient>(this, &MQTTClient::callback);
  this->hostname = _hostname;
  this->port = _port;
  this->options = MQTTPacket_connectData_initializer;

  return true;
}

MQTTClient MQTTClient::callback(MQTT::MessageData& messageData) {
    MQTT::Message &message = messageData.message;
    // null terminate topic to create String object
    int len = messageData.topicName.lenstring.len;
    char topic[len+1];
    memcpy(topic, messageData.topicName.lenstring.data, (size_t)len);
    topic[len] = '\0';

    // null terminate payload
    char * payload = (char *)message.payload;
    payload[message.payloadlen] = '\0';
    messageReceived(String(topic), String(payload), (char*)message.payload, (unsigned int)message.payloadlen);
}

void MQTTClient::setWill(const char * topic) {
  this->setWill(topic, "");
}

void MQTTClient::setWill(const char * topic, const char * payload) {
  this->options.willFlag = 0x1;
  this->options.will.topicName.cstring = (char*)topic;
  this->options.will.message.cstring = (char*)payload;
}

boolean MQTTClient::connect(const char * clientId) {
  return this->connect(clientId, NULL, NULL);
}

boolean MQTTClient::connect(const char * clientId, const char * username, const char * password) {
  if(!this->network.connect((char*)this->hostname, this->port)) {
    return false;
  }

  this->options.clientID.cstring = (char*)clientId;
  if(username && password) {
    this->options.username.cstring = (char*)username;
    this->options.password.cstring = (char*)password;
  }

  return this->client->connect(this->options) == 0;
}

void MQTTClient::publish(String topic) {
  this->publish(topic.c_str(), "");
}

void MQTTClient::publish(String topic, String payload) {
  this->publish(topic.c_str(), payload.c_str());
}

void MQTTClient::publish(const char * topic, String payload) {
  this->publish(topic, payload.c_str());
}

void MQTTClient::publish(const char * topic, const char * payload) {
  this->publish(topic, (char*)payload, (unsigned int)strlen(payload));
}

void MQTTClient::publish(const char * topic, char * payload, unsigned int length) {
  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;
  message.payload = payload;
  message.payloadlen = length;
  client->publish(topic, message);
}

void MQTTClient::subscribe(String topic) {
  this->subscribe(topic.c_str());
}

void MQTTClient::subscribe(const char * topic) {
  client->subscribe(topic, MQTT::QOS0, NULL);
}

void MQTTClient::unsubscribe(String topic) {
  this->unsubscribe(topic.c_str());
}

void MQTTClient::unsubscribe(const char * topic) {
  client->unsubscribe(topic);
}

void MQTTClient::loop() {
  if(!this->network.connected() && this->client->isConnected()) {
    // the following call will not send a packet but reset the instance
    // to allow proper reconnection
    this->client->disconnect();
  }

  this->client->yield();
}

boolean MQTTClient::connected() {
  return this->client->isConnected();
}

void MQTTClient::disconnect() {
  this->client->disconnect();
}
