#include <stddef.h>

#include "lwmqtt.h"

#include "packet.h"

void lwmqtt_init(lwmqtt_client_t *client, void *write_buf, int write_buf_size, void *read_buf, int read_buf_size) {
  client->last_packet_id = 1;
  client->keep_alive_interval = 0;
  client->ping_outstanding = false;

  client->write_buf = write_buf;
  client->write_buf_size = write_buf_size;
  client->read_buf = read_buf;
  client->read_buf_size = read_buf_size;
  client->callback = NULL;

  client->network = NULL;
  client->network_read = NULL;
  client->network_write = NULL;

  client->keep_alive_timer = NULL;
  client->command_timer = NULL;
  client->timer_set = NULL;
  client->timer_get = NULL;
}

void lwmqtt_set_network(lwmqtt_client_t *client, void *ref, lwmqtt_network_read_t read, lwmqtt_network_write_t write) {
  client->network = ref;
  client->network_read = read;
  client->network_write = write;
}

void lwmqtt_set_timers(lwmqtt_client_t *client, void *keep_alive_timer, void *network_timer, lwmqtt_timer_set_t set,
                       lwmqtt_timer_get_t get) {
  client->keep_alive_timer = keep_alive_timer;
  client->command_timer = network_timer;
  client->timer_set = set;
  client->timer_get = get;

  client->timer_set(client, client->keep_alive_timer, 0);
  client->timer_set(client, client->command_timer, 0);
}

void lwmqtt_set_callback(lwmqtt_client_t *client, void *ref, lwmqtt_callback_t cb) {
  client->callback_ref = ref;
  client->callback = cb;
}

static long lwmqtt_get_next_packet_id(lwmqtt_client_t *client) {
  return client->last_packet_id = (client->last_packet_id == 65535) ? 1 : client->last_packet_id + 1;
}

static lwmqtt_err_t lwmqtt_read_from_network(lwmqtt_client_t *client, int offset, int len) {
  // check read buffer capacity
  if (client->read_buf_size < offset + len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // prepare counter
  int read = 0;

  // read while data is missing
  while (read < len) {
    // get remaining time
    int remaining_time = client->timer_get(client, client->command_timer);

    // check timeout
    if (remaining_time <= 0) {
      return LWMQTT_NOT_ENOUGH_DATA;
    }

    // read
    int partial_read = 0;
    lwmqtt_err_t err =
        client->network_read(client, client->network, client->read_buf + offset + read, len - read, &partial_read, remaining_time);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // increment counter
    read += partial_read;
  }

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t lwmqtt_write_to_network(lwmqtt_client_t *client, int offset, int len) {
  // prepare counter
  int written = 0;

  // write while data is left
  while (written < len) {
    // get remaining time
    int remaining_time = client->timer_get(client, client->command_timer);

    // check timeout
    if (remaining_time <= 0) {
      return LWMQTT_NOT_ENOUGH_DATA;
    }

    // read
    int partial_write = 0;
    lwmqtt_err_t err =
        client->network_write(client, client->network, client->write_buf + offset + written, len - written, &partial_write, remaining_time);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // increment counter
    written += partial_write;
  }

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t lwmqtt_read_packet_in_buffer(lwmqtt_client_t *client, int *read, lwmqtt_packet_type_t *packet_type) {
  // preset packet type
  *packet_type = LWMQTT_NO_PACKET;

  // read or wait for header byte
  lwmqtt_err_t err = lwmqtt_read_from_network(client, 0, 1);
  if (err == LWMQTT_NOT_ENOUGH_DATA) {
    return LWMQTT_SUCCESS;
  } else if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // detect packet type
  err = lwmqtt_detect_packet_type(client->read_buf, packet_type);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // prepare variables
  int len = 0;
  long rem_len = 0;

  do {
    // adjust len
    len++;

    // read next byte
    err = lwmqtt_read_from_network(client, len, 1);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // attempt to detect remaining length
    err = lwmqtt_detect_remaining_length(client->read_buf + 1, len, &rem_len);
  } while (err == LWMQTT_BUFFER_TOO_SHORT);

  // check final error
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read the rest of the buffer if needed
  if (rem_len > 0) {
    err = lwmqtt_read_from_network(client, 1 + len, (int)rem_len);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // adjust counter
  *read += 1 + len + rem_len;

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t lwmqtt_send_packet_in_buffer(lwmqtt_client_t *client, int length) {
  // write to network
  lwmqtt_err_t err = lwmqtt_write_to_network(client, 0, length);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // reset keep alive timer
  client->timer_set(client, client->keep_alive_timer, client->keep_alive_interval * 1000);

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t lwmqtt_cycle(lwmqtt_client_t *client, int *read, lwmqtt_packet_type_t *packet_type) {
  // read next packet from the network
  lwmqtt_err_t err = lwmqtt_read_packet_in_buffer(client, read, packet_type);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (*packet_type == LWMQTT_NO_PACKET) {
    return LWMQTT_SUCCESS;
  }

  switch (*packet_type) {
    // handle publish packets
    case LWMQTT_PUBLISH_PACKET: {
      // decode publish packet
      lwmqtt_string_t topic = lwmqtt_default_string;
      lwmqtt_message_t msg;
      bool dup;
      long packet_id;
      err = lwmqtt_decode_publish(client->read_buf, client->read_buf_size, &dup, &msg.qos, &msg.retained, &packet_id, &topic,
                                  &msg.payload, &msg.payload_len);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      // call callback if set
      if (client->callback != NULL) {
        client->callback(client, client->callback_ref, &topic, &msg);
      }

      // break early on qos zero
      if (msg.qos == LWMQTT_QOS0) {
        break;
      }

      // define ack packet
      lwmqtt_packet_type_t ack_type = LWMQTT_NO_PACKET;
      if (msg.qos == LWMQTT_QOS1) {
        ack_type = LWMQTT_PUBREC_PACKET;
      } else if (msg.qos == LWMQTT_QOS2) {
        ack_type = LWMQTT_PUBREL_PACKET;
      }

      // encode ack packet
      int len;
      err = lwmqtt_encode_ack(client->write_buf, client->write_buf_size, &len, ack_type, false, packet_id);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      // send ack packet
      err = lwmqtt_send_packet_in_buffer(client, len);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      break;
    }

    // handle pubrec packets
    case LWMQTT_PUBREC_PACKET: {
      // decode pubrec packet
      bool dup;
      long packet_id;
      err = lwmqtt_decode_ack(client->read_buf, client->read_buf_size, packet_type, &dup, &packet_id);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      // encode pubrel packet
      int len;
      err = lwmqtt_encode_ack(client->write_buf, client->write_buf_size, &len, LWMQTT_PUBREL_PACKET, 0, packet_id);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      // send pubrel packet
      err = lwmqtt_send_packet_in_buffer(client, len);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      break;
    }

    // handle pubrel packets
    case LWMQTT_PUBREL_PACKET: {
      // decode pubrec packet
      bool dup;
      long packet_id;
      err = lwmqtt_decode_ack(client->read_buf, client->read_buf_size, packet_type, &dup, &packet_id);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      // encode pubcomp packet
      int len;
      err = lwmqtt_encode_ack(client->write_buf, client->write_buf_size, &len, LWMQTT_PUBCOMP_PACKET, 0, packet_id);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      // send pubcomp packet
      err = lwmqtt_send_packet_in_buffer(client, len);
      if (err != LWMQTT_SUCCESS) {
        return err;
      }

      break;
    }

    // handle pingresp packets
    case LWMQTT_PINGRESP_PACKET: {
      // set flag
      client->ping_outstanding = false;

      break;
    }

    // handle all other packets
    default: { break; }
  }

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t lwmqtt_cycle_until(lwmqtt_client_t *client, lwmqtt_packet_type_t *packet_type, int available,
                                       lwmqtt_packet_type_t needle) {
  // prepare counter
  int read = 0;

  // loop until timeout has been reached
  do {
    // do one cycle
    lwmqtt_err_t err = lwmqtt_cycle(client, &read, packet_type);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // check if needle has been found
    if (needle != LWMQTT_NO_PACKET && *packet_type == needle) {
      return LWMQTT_SUCCESS;
    }
  } while (client->timer_get(client, client->command_timer) > 0 && (available == 0 || read < available));

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_yield(lwmqtt_client_t *client, int available, int timeout) {
  // set timeout
  client->timer_set(client, client->command_timer, timeout);

  // cycle until timeout has been reached
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  lwmqtt_err_t err = lwmqtt_cycle_until(client, &packet_type, available, LWMQTT_NO_PACKET);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_connect(lwmqtt_client_t *client, lwmqtt_options_t *options, lwmqtt_will_t *will,
                            lwmqtt_return_code_t *return_code, int timeout) {
  // set timer to command timeout
  client->timer_set(client, client->command_timer, timeout);

  // save keep alive interval
  client->keep_alive_interval = options->keep_alive;

  // set keep alive timer
  if (client->keep_alive_interval > 0) {
    client->timer_set(client, client->keep_alive_timer, client->keep_alive_interval * 1000);
  }

  // encode connect packet
  int len;
  lwmqtt_err_t err = lwmqtt_encode_connect(client->write_buf, client->write_buf_size, &len, options, will);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // send packet
  err = lwmqtt_send_packet_in_buffer(client, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // wait for connack packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(client, &packet_type, 0, LWMQTT_CONNACK_PACKET);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (packet_type != LWMQTT_CONNACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode connack packet
  bool session_present;
  err = lwmqtt_decode_connack(client->read_buf, client->read_buf_size, &session_present, return_code);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // return error if connection was not accepted
  if (*return_code != LWMQTT_CONNACK_CONNECTION_ACCEPTED) {
    return LWMQTT_CONNECTION_DENIED;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_subscribe(lwmqtt_client_t *client, int count, const char **topic_filter, lwmqtt_qos_t *qos,
                              int timeout) {
  // set timeout
  client->timer_set(client, client->command_timer, timeout);

  // prepare string objects
  lwmqtt_string_t strings[count];

  // convert strings
  for (int i = 0; i < count; i++) {
    strings[i] = lwmqtt_str(topic_filter[i]);
  }

  // encode subscribe packet
  int len;
  lwmqtt_err_t err = lwmqtt_encode_subscribe(client->write_buf, client->write_buf_size, &len,
                                             lwmqtt_get_next_packet_id(client), count, strings, qos);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // send packet
  err = lwmqtt_send_packet_in_buffer(client, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // wait for suback packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(client, &packet_type, 0, LWMQTT_SUBACK_PACKET);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (packet_type != LWMQTT_SUBACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode packet
  int suback_count = 0;
  lwmqtt_qos_t granted_qos[count];
  long packet_id;
  err = lwmqtt_decode_suback(client->read_buf, client->read_buf_size, &packet_id, count, &suback_count, granted_qos);
  if (err == LWMQTT_SUCCESS) {
    return err;
  }

  // check suback codes
  for (int i = 0; i < suback_count; i++) {
    if (granted_qos[i] == LWMQTT_QOS_FAILURE) {
      return LWMQTT_FAILED_SUBSCRIPTION;
    }
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_subscribe_one(lwmqtt_client_t *client, const char *topic_filter, lwmqtt_qos_t qos, int timeout) {
  return lwmqtt_subscribe(client, 1, &topic_filter, &qos, timeout);
}

lwmqtt_err_t lwmqtt_unsubscribe(lwmqtt_client_t *client, int count, const char **topic_filter, int timeout) {
  // set timer
  client->timer_set(client, client->command_timer, timeout);

  // prepare string objects
  lwmqtt_string_t strings[count];

  // convert strings
  for (int i = 0; i < count; i++) {
    strings[i] = lwmqtt_str(topic_filter[i]);
  }

  // encode unsubscribe packet
  int len;
  lwmqtt_err_t err = lwmqtt_encode_unsubscribe(client->write_buf, client->write_buf_size, &len,
                                               lwmqtt_get_next_packet_id(client), count, strings);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // send unsubscribe packet
  err = lwmqtt_send_packet_in_buffer(client, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // wait for unsuback packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(client, &packet_type, 0, LWMQTT_UNSUBACK_PACKET);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (packet_type != LWMQTT_UNSUBACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode unsuback packet
  bool dup;
  long packet_id;
  err = lwmqtt_decode_ack(client->read_buf, client->read_buf_size, &packet_type, &dup, &packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_unsubscribe_one(lwmqtt_client_t *client, const char *topic_filter, int timeout) {
  return lwmqtt_unsubscribe(client, 1, &topic_filter, timeout);
}

lwmqtt_err_t lwmqtt_publish(lwmqtt_client_t *client, const char *topic, lwmqtt_message_t *message, int timeout) {
  // prepare string
  lwmqtt_string_t str = lwmqtt_str(topic);

  // set timer
  client->timer_set(client, client->command_timer, timeout);

  // add packet id if at least qos 1
  long packet_id = 0;
  if (message->qos == LWMQTT_QOS1 || message->qos == LWMQTT_QOS2) {
    packet_id = lwmqtt_get_next_packet_id(client);
  }

  // encode publish packet
  int len = 0;
  lwmqtt_err_t err =
      lwmqtt_encode_publish(client->write_buf, client->write_buf_size, &len, 0, message->qos,
                            (char)(message->retained ? 1 : 0), packet_id, str, message->payload, message->payload_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // send packet
  err = lwmqtt_send_packet_in_buffer(client, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // immediately return on qos zero
  if (message->qos == LWMQTT_QOS0) {
    return LWMQTT_SUCCESS;
  }

  // define ack packet
  lwmqtt_packet_type_t ack_type = LWMQTT_NO_PACKET;
  if (message->qos == LWMQTT_QOS1) {
    ack_type = LWMQTT_PUBACK_PACKET;
  } else if (message->qos == LWMQTT_QOS2) {
    ack_type = LWMQTT_PUBCOMP_PACKET;
  }

  // wait for ack packet
  lwmqtt_packet_type_t packet_type = LWMQTT_NO_PACKET;
  err = lwmqtt_cycle_until(client, &packet_type, 0, ack_type);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (packet_type != ack_type) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // decode ack packet
  bool dup;
  err = lwmqtt_decode_ack(client->read_buf, client->read_buf_size, &packet_type, &dup, &packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_disconnect(lwmqtt_client_t *client, int timeout) {
  // set timer
  client->timer_set(client, client->command_timer, timeout);

  // encode disconnect packet
  int len;
  lwmqtt_err_t err = lwmqtt_encode_zero(client->write_buf, client->write_buf_size, &len, LWMQTT_DISCONNECT_PACKET);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // send disconnected packet
  err = lwmqtt_send_packet_in_buffer(client, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_keep_alive(lwmqtt_client_t *client, int timeout) {
  // set timer
  client->timer_set(client, client->command_timer, timeout);

  // return immediately if keep alive interval is zero
  if (client->keep_alive_interval == 0) {
    return LWMQTT_SUCCESS;
  }

  // return immediately if no ping is due
  if (client->timer_get(client, client->keep_alive_timer) > 0) {
    return LWMQTT_SUCCESS;
  }

  // a ping is due

  // fail immediately if a ping is still outstanding
  if (client->ping_outstanding) {
    return LWMQTT_UNANSWERED_PING;
  }

  // encode pingreq packet
  int len;
  lwmqtt_err_t err = lwmqtt_encode_zero(client->write_buf, client->write_buf_size, &len, LWMQTT_PINGREQ_PACKET);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // send packet
  err = lwmqtt_send_packet_in_buffer(client, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set flag
  client->ping_outstanding = true;

  return LWMQTT_SUCCESS;
}
