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

lwmqtt_err_t lwmqtt_arduino_network_read(lwmqtt_client_t *client, void *ref, unsigned char *buffer, int len, int *read,
                                         unsigned int timeout) {
  // cast network reference
  lwmqtt_arduino_network_t *n = (lwmqtt_arduino_network_t *)ref;

  // set timeout
  n->client->setTimeout(timeout);

  // read bytes
  *read = (int)n->client->readBytes(buffer, (size_t)len);
  if (*read <= 0) {
    return LWMQTT_NETWORK_READ_ERROR;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_arduino_network_write(lwmqtt_client_t *client, void *ref, unsigned char *buffer, int len, int *sent,
                                          unsigned int timeout) {
  // cast network reference
  lwmqtt_arduino_network_t *n = (lwmqtt_arduino_network_t *)ref;

  // write bytes
  *sent = (int)n->client->write(buffer, (size_t)len);
  if (*sent <= 0) {
    return LWMQTT_NETWORK_WRITE_ERR;
  };

  return LWMQTT_SUCCESS;
}
