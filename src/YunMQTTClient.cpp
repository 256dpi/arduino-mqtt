#include "YunMQTTClient.h"

//void messageArrived(MQTT::MessageData& messageData) {
//  MQTT::Message &message = messageData.message;
//
//  // null terminate topic to create String object
//  int len = messageData.topicName.lenstring.len;
//  char topic[len+1];
//  memcpy(topic, messageData.topicName.lenstring.data, (size_t)len);
//  topic[len] = '\0';
//
//  // null terminate payload
//  char * payload = (char *)message.payload;
//  payload[message.payloadlen] = '\0';
//  messageReceived(String(topic), String(payload), (char*)message.payload, (unsigned int)message.payloadlen);
//}

YunMQTTClient::YunMQTTClient(const char * _hostname, int _port, Client& _client) {
//  this->client = new MQTT::Client<Network, Timer, MQTT_BUFFER_SIZE, 0>(this->network);
//  this->network.setClient(&_client);
//  this->client->setDefaultMessageHandler(messageArrived);
//  this->hostname = _hostname;
//  this->port = _port;
}

boolean YunMQTTClient::connect(const char * clientId) {
//  return this->connect(clientId, "", "");
}

boolean YunMQTTClient::connect(const char * clientId, const char * username, const char * password) {
//  if(!this->network.connect((char*)this->hostname, this->port)) {
//    return false;
//  }
//
//  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
//  data.clientID.cstring = (char*)clientId;
//  if(username && password) {
//    data.username.cstring = (char*)username;
//    data.password.cstring = (char*)password;
//  }
//
//  return this->client->connect(data) == 0;
}

boolean YunMQTTClient::publish(String topic) {
  return this->publish(topic.c_str(), "");
}

boolean YunMQTTClient::publish(String topic, String payload) {
  return this->publish(topic.c_str(), payload.c_str());
}

boolean YunMQTTClient::publish(const char * topic, String payload) {
//  return this->publish(topic, payload.c_str());
}

boolean YunMQTTClient::publish(const char * topic, const char * payload) {
//  MQTT::Message message;
//  message.qos = MQTT::QOS0;
//  message.retained = false;
//  message.dup = false;
//  message.payload = (char*)payload;
//  message.payloadlen = strlen(payload);
//  return client->publish(topic, message) == 0;
}

boolean YunMQTTClient::subscribe(String topic) {
  return this->subscribe(topic.c_str());
}

boolean YunMQTTClient::subscribe(const char * topic) {
//  return client->subscribe(topic, MQTT::QOS0, NULL) == 0;
}

boolean YunMQTTClient::unsubscribe(String topic) {
  return this->unsubscribe(topic.c_str());
}

boolean YunMQTTClient::unsubscribe(const char * topic) {
//  return client->unsubscribe(topic) == 0;
}

boolean YunMQTTClient::loop() {
//  return this->client->yield() == 0;
}

boolean YunMQTTClient::connected() {
//  return this->client->isConnected();
}

void YunMQTTClient::disconnect() {
//  this->client->disconnect();
}
