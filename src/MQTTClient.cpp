#include "MQTTClient.h"

void MQTTClient_messageHandler(MQTT::MessageData &messageData) {
  MQTT::Message &message = messageData.message;

  // null terminate topic to create String object
  int len = messageData.topicName.lenstring.len; 
  char topic[len+1];
  memcpy(topic, messageData.topicName.lenstring.data, (size_t)len);
  topic[len] = '\0';

  // get payload
  char * payload = (char *)message.payload;

  // null terminate payload if enough space is available
  if(message.payloadlen < MQTT_BUFFER_SIZE) {
    payload[message.payloadlen] = '\0';
  }

  messageReceived(String(topic), String(payload), (char*)message.payload, (unsigned int)message.payloadlen);
}

MQTTClient::MQTTClient() {}

boolean MQTTClient::begin(const char * hostname, Client& client) {
  return this->begin(hostname, 1883, client);
}

boolean MQTTClient::begin(const char * _hostname, int _port, Client& _client) {
  this->client = new MQTT::Client<Network, Timer, MQTT_BUFFER_SIZE, 0>(this->network);
  this->network.setClient(&_client);
  this->client->setDefaultMessageHandler(MQTTClient_messageHandler);
  this->hostname = _hostname;
  this->port = _port;
  this->options = MQTTPacket_connectData_initializer;

  return true;
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
  
  return this->client->connect(this->options) == MQTT::SUCCESS;
}

boolean MQTTClient::publish(String topic) {
  return this->publish(topic.c_str(), "");
}

boolean MQTTClient::publish(String topic, String payload) {
  return this->publish(topic.c_str(), payload.c_str());
}

boolean MQTTClient::publish(const char * topic, String payload) {
  return this->publish(topic, payload.c_str());
}

boolean MQTTClient::publish(const char * topic, const char * payload) {
  return this->publish(topic, (char*)payload, (unsigned int)strlen(payload));
}

boolean MQTTClient::publish(const char * topic, char * payload, unsigned int length) {
  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;
  message.payload = payload;
  message.payloadlen = length;

  return client->publish(topic, message) == MQTT::SUCCESS;
}

boolean MQTTClient::publish(MQTTMessage * message) {
  MQTT::Message _message;
  _message.qos = MQTT::QOS0;
  _message.retained = message->retained;
  _message.dup = false;
  _message.payload = message->payload;
  _message.payloadlen = message->length;

  return client->publish(message->topic, _message) == MQTT::SUCCESS;
}

boolean MQTTClient::subscribe(String topic) {
  return this->subscribe(topic.c_str());
}

boolean MQTTClient::subscribe(const char * topic) {
  return client->subscribe(topic, MQTT::QOS0, NULL) == MQTT::SUCCESS;
}

boolean MQTTClient::unsubscribe(String topic) {
  return this->unsubscribe(topic.c_str());
}

boolean MQTTClient::unsubscribe(const char * topic) {
  return client->unsubscribe(topic) == MQTT::SUCCESS;
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

boolean MQTTClient::disconnect() {
  return this->client->disconnect() == MQTT::SUCCESS;
}
