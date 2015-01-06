#include <Arduino.h>
#include "Network.h"

void Network::setClient(Client * _client) {
  this->client = _client;
}

int Network::connect(char* hostname, int port) {
  return this->client->connect(hostname, port);
}

int Network::read(unsigned char* buffer, int len, int timeout) {
	if(this->client->available() > 0) {
		this->client->setTimeout(timeout);
		return this->client->readBytes(buffer, len);
	} else {
		return 0; // immediately return if there is nothing to read
	}
}
    
int Network::write(unsigned char* buffer, int len, int timeout) {
  client->setTimeout(timeout);  
  return client->write((uint8_t*)buffer, len);
}
    
int Network::disconnect() {
  client->stop();
  return 0;
}
