#include <MQTTClient.h>

void messageArrived(MQTT::MessageData& messageData) {
  MQTT::Message &message = messageData.message;
  char * topic = messageData.topicName.cstring;
  char * payload = (char*)message.payload;
  messageReceived(String(topic), String(payload));
}

MQTTClient::MQTTClient(const char * _hostname, int _port, Client& _client) {
  this->network.setClient(&_client);
  this->client = new MQTT::Client<Network, Timer>(this->network);
  this->hostname = _hostname;
  this->port = _port;
}

boolean MQTTClient::connect(const char * clientId) {
  return this->connect(clientId, "", "");
}

boolean MQTTClient::connect(const char * clientId, const char * _username, const char * _password) {
  if(!this->network.connect((char*)this->hostname, this->port)) {
    return false;
  }
  
  MQTTString username;
  username.cstring = (char*)_username;
  MQTTString password;
  password.cstring = (char*)_password;
  
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.MQTTVersion = 3;
  data.username = username;
  data.password = password;
  data.clientID.cstring = (char*)clientId;
  return this->client->connect(data) == 0;
}

boolean MQTTClient::publish(const char * topic, String payload) {
  char * buf = (char*)malloc(payload.length());
  payload.toCharArray(buf, payload.length());
  return this->publish(topic, buf);
  free(buf);
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
