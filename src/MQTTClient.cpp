#include <MQTTClient.h>

void messageArrived(MQTT::MessageData& messageData) {
  MQTT::Message &message = messageData.message;
  char * topic = messageData.topicName.lenstring.data;
  messageReceived(String(topic), (char*)message.payload, message.payloadlen);
}

MQTTClient::MQTTClient(const char * _hostname, int _port, Client& _client) {
  this->network.setClient(&_client);
  //TODO: find a way to use dynamic allocation
  this->client = new MQTT::Client<Network, Timer, MQTT_BUFFER_SIZE>(this->network);
  this->hostname = _hostname;
  this->port = _port;
}

boolean MQTTClient::connect(const char * clientId) {
  return this->connect(clientId, "", "");
}

boolean MQTTClient::connect(const char * clientId, const char * username, const char * password) {
  if(!this->network.connect((char*)this->hostname, this->port)) {
    return false;
  }
  
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.clientID.cstring = (char*)clientId;
  if(username && password) {
    data.username.cstring = (char*)username;
    data.password.cstring = (char*)password;
  }
  
  return this->client->connect(data) == 0;
}

boolean MQTTClient::publish(const char * topic, String payload) {
  return this->publish(topic, payload.c_str());
}

boolean MQTTClient::publish(const char * topic, const char * payload) {
  MQTT::Message message;
  message.qos = MQTT::QOS0;
  message.retained = false;
  message.dup = false;
  message.payload = (char*)payload;
  message.payloadlen = strlen(payload);
  return client->publish(topic, message) == 0;
}

boolean MQTTClient::subscribe(const char * topic) {
  return client->subscribe(topic, MQTT::QOS0, messageArrived) == 0;
}

boolean MQTTClient::unsubscribe(const char * topic) {
  return client->unsubscribe(topic) == 0;
}
  
boolean MQTTClient::loop() {
  this->client->yield();
}

boolean MQTTClient::connected() {
  return this->client->isConnected();
}

void MQTTClient::disconnect() {
  this->client->disconnect();
}
