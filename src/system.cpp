#include <Arduino.h>

#include "system.h"

void lwmqtt_arduino_timer_set(lwmqtt_client_t *client, void *ref, unsigned int timeout) {
  // cast timer reference
  lwmqtt_arduino_timer_t *t = (lwmqtt_arduino_timer_t *)ref;

  // set future end time
  t->end = millis() + timeout;
}

unsigned int lwmqtt_arduino_timer_get(lwmqtt_client_t *client, void *ref) {
  // cast timer reference
  lwmqtt_arduino_timer_t *t = (lwmqtt_arduino_timer_t *)ref;

  // get difference to end time
  return (unsigned int)(t->end - millis());
}

lwmqtt_err_t lwmqtt_arduino_network_connect(lwmqtt_arduino_network_t *network, Client *client, char *host, int port) {
  // save client
  network->client = client;

  // connect to host
  int rc = network->client->connect(host, (uint16_t)port);
  if (rc < 0) {
    return LWMQTT_FAILURE;
  }

  return LWMQTT_SUCCESS;
}

void lwmqtt_arduino_network_disconnect(lwmqtt_arduino_network_t *network) { network->client->stop(); }

lwmqtt_err_t lwmqtt_arduino_network_peek(lwmqtt_client_t *client, lwmqtt_arduino_network_t *network, int *available) {
  *available = network->client->available();
  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_arduino_network_read(lwmqtt_client_t *client, void *ref, unsigned char *buffer, int len, int *read,
                                         unsigned int timeout) {
  // cast network reference
  lwmqtt_arduino_network_t *n = (lwmqtt_arduino_network_t *)ref;

  if (!n->client->connected()) {
    return LWMQTT_FAILURE;
  }

  *read = n->client->available();
  if (*read == 0) {
    return LWMQTT_SUCCESS;
  }

  // set timeout
  n->client->setTimeout(timeout);

  // read bytes
  if (n->client->readBytes(buffer, (size_t)len) <= 0) {
    return LWMQTT_FAILURE;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_arduino_network_write(lwmqtt_client_t *client, void *ref, unsigned char *buffer, int len, int *sent,
                                          unsigned int timeout) {
  // cast network reference
  lwmqtt_arduino_network_t *n = (lwmqtt_arduino_network_t *)ref;

  // check if still connected
  if (!n->client->connected()) {
    return LWMQTT_FAILURE;
  }

  // set timeout
  n->client->setTimeout(timeout);

  // write bytes
  if (n->client->write(buffer, (size_t)len) <= 0) {
    return LWMQTT_FAILURE;
  };

  return LWMQTT_SUCCESS;
}
