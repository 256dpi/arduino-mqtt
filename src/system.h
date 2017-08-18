#ifndef LWMQTT_ARDUINO_H
#define LWMQTT_ARDUINO_H

#include <Arduino.h>
#include <Client.h>

extern "C" {
#include "lwmqtt/lwmqtt.h"
};

typedef struct { unsigned long end; } lwmqtt_arduino_timer_t;

void lwmqtt_arduino_timer_set(lwmqtt_client_t *client, void *ref, int timeout);

int lwmqtt_arduino_timer_get(lwmqtt_client_t *client, void *ref);

typedef struct { Client *client; } lwmqtt_arduino_network_t;

lwmqtt_err_t lwmqtt_arduino_network_read(lwmqtt_client_t *client, void *ref, void *buf, int len, int *read,
                                         int timeout);
lwmqtt_err_t lwmqtt_arduino_network_write(lwmqtt_client_t *client, void *ref, void *buf, int len, int *sent,
                                          int timeout);

#endif  // LWMQTT_ARDUINO_H
