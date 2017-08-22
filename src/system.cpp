#include <Arduino.h>

#include "system.h"

void lwmqtt_arduino_timer_set(lwmqtt_client_t *client, void *ref, int timeout) {
  // cast timer reference
  auto *t = (lwmqtt_arduino_timer_t *)ref;

  // set future end time
  t->end = millis() + timeout;
}

int lwmqtt_arduino_timer_get(lwmqtt_client_t *client, void *ref) {
  // cast timer reference
  auto *t = (lwmqtt_arduino_timer_t *)ref;

  // get difference to end time
  return (int)(t->end - millis());
}

lwmqtt_err_t lwmqtt_arduino_network_read(lwmqtt_client_t *client, void *ref, uint8_t *buffer, size_t len, size_t *read,
                                         int timeout) {
  // cast network reference
  auto *n = (lwmqtt_arduino_network_t *)ref;

  // set timeout
  n->client->setTimeout((unsigned long)timeout);

  // read bytes
  *read = n->client->readBytes(buffer, len);
  if (*read <= 0) {
    return LWMQTT_NETWORK_FAILED_READ;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_arduino_network_write(lwmqtt_client_t *client, void *ref, uint8_t *buffer, size_t len, size_t *sent,
                                          int timeout) {
  // cast network reference
  auto *n = (lwmqtt_arduino_network_t *)ref;

  // write bytes
  *sent = n->client->write(buffer, len);
  if (*sent <= 0) {
    return LWMQTT_NETWORK_FAILED_WRITE;
  };

  return LWMQTT_SUCCESS;
}
