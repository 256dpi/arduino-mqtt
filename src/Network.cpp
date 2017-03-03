#include "Network.h"

void Network::setClient(Client *_client) { this->client = _client; }

int Network::connect(char *hostname, int port) {
  return this->client->connect(hostname, (uint16_t)port);
}

int Network::read(unsigned char *buffer, int len, int timeout) {
  if (!client->connected()) {
    return -1; // return an error
  }

  if (this->client->available() == 0) {
    return 0; // nothing to read
  }

  this->client->setTimeout((unsigned long)timeout);
  return (int)this->client->readBytes(buffer, (size_t)len);
}

int Network::write(unsigned char *buffer, int len, int timeout) {
  if (!client->connected()) {
    return -1; // return an error
  }

  client->setTimeout((unsigned long)timeout);
  return (int)client->write(buffer, (size_t)len);
}

boolean Network::connected() { return client->connected(); }

int Network::disconnect() {
  client->stop();
  return 0;
}
