#ifndef LWMQTT_ARDUINO_H
#define LWMQTT_ARDUINO_H

#include <Arduino.h>
#include <Client.h>

extern "C" {
#include "lwmqtt/lwmqtt.h"
};

typedef struct { unsigned long end; } lwmqtt_arduino_timer_t;

void lwmqtt_arduino_timer_set(lwmqtt_client_t *client, void *ref, unsigned int timeout);

unsigned int lwmqtt_arduino_timer_get(lwmqtt_client_t *client, void *ref);

typedef struct { Client *client; } lwmqtt_arduino_network_t;

lwmqtt_err_t lwmqtt_arduino_network_connect(lwmqtt_arduino_network_t *network, Client *client, char *host, int port);

void lwmqtt_arduino_network_disconnect(lwmqtt_arduino_network_t *network);

lwmqtt_err_t lwmqtt_arduino_network_peek(lwmqtt_client_t *client, lwmqtt_arduino_network_t *network, int *available);

lwmqtt_err_t lwmqtt_arduino_network_read(lwmqtt_client_t *client, void *ref, unsigned char *buf, int len, int *read,
                                         unsigned int timeout);
lwmqtt_err_t lwmqtt_arduino_network_write(lwmqtt_client_t *client, void *ref, unsigned char *buf, int len, int *sent,
                                          unsigned int timeout);

#endif  // LWMQTT_ARDUINO_H
