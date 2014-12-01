#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "Network.h"

void Network::setClient(Client * _client) {
  this->client = _client;
}

int Network::connect(char* hostname, int port) {
  return this->client->connect(hostname, port);
}

int Network::read(unsigned char* buffer, int len, int timeout) {
  int interval = 10;  // all times are in milliseconds
	int total = 0;

	if (timeout < 30) {
	  interval = 2;
	}
    
	while (client->available() < len && total < timeout) {
		delay(interval);
		total += interval;
	}
    
	if (client->available() >= len) {
		return client->readBytes((char*)buffer, len);;
	}
			
	return -1;
}
    
int Network::write(unsigned char* buffer, int len, int timeout) {
  client->setTimeout(timeout);  
  return client->write((uint8_t*)buffer, len);
}
    
int Network::disconnect() {
  client->stop();
  return 0;
}

#endif
