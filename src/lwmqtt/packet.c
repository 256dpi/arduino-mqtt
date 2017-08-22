#include "packet.h"

typedef union {
  uint8_t byte;
  struct {
    unsigned int retain : 1;
    unsigned int qos : 2;
    unsigned int dup : 1;
    unsigned int type : 4;
  } bits;
} lwmqtt_header_t;

typedef union {
  uint8_t byte;
  struct {
    unsigned int _ : 1;
    unsigned int clean_session : 1;
    unsigned int will : 1;
    unsigned int will_qos : 2;
    unsigned int will_retain : 1;
    unsigned int password : 1;
    unsigned int username : 1;
  } bits;
} lwmqtt_connect_flags_t;

typedef union {
  uint8_t byte;
  struct {
    unsigned int _ : 7;
    unsigned int session_present : 1;
  } bits;
} lwmqtt_connack_flags_t;

lwmqtt_err_t lwmqtt_detect_packet_type(uint8_t *buf, size_t buf_len, lwmqtt_packet_type_t *packet_type) {
  // set default packet type
  *packet_type = LWMQTT_NO_PACKET;

  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // prepare header
  lwmqtt_header_t header;

  // reader header
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set packet type
  *packet_type = (lwmqtt_packet_type_t)header.bits.type;

  // check if packet type is correct and can be received
  switch (*packet_type) {
    case LWMQTT_CONNACK_PACKET:
    case LWMQTT_PUBLISH_PACKET:
    case LWMQTT_PUBACK_PACKET:
    case LWMQTT_PUBREC_PACKET:
    case LWMQTT_PUBREL_PACKET:
    case LWMQTT_PUBCOMP_PACKET:
    case LWMQTT_SUBACK_PACKET:
    case LWMQTT_UNSUBACK_PACKET:
    case LWMQTT_PINGRESP_PACKET:
      return LWMQTT_SUCCESS;
    default:
      *packet_type = LWMQTT_NO_PACKET;
      return LWMQTT_MISSING_OR_WRONG_PACKET;
  }
}

lwmqtt_err_t lwmqtt_detect_remaining_length(uint8_t *buf, size_t buf_len, uint32_t *rem_len) {
  // prepare pointer
  uint8_t *ptr = buf;

  // attempt to decode remaining length
  lwmqtt_err_t err = lwmqtt_read_varnum(&ptr, buf + buf_len, rem_len);
  if (err == LWMQTT_VARNUM_OVERFLOW) {
    *rem_len = 0;
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  } else if (err != LWMQTT_SUCCESS) {
    *rem_len = 0;
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_connect(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_options_t options,
                                   lwmqtt_will_t *will) {
  // prepare pointers
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // fixed header is 10
  uint32_t rem_len = 10;

  // add client id to remaining length
  rem_len += options.client_id.len + 2;

  // add will if present to remaining length
  if (will != NULL) {
    rem_len += will->topic.len + 2 + will->payload.len + 2;
  }

  // add username if present to remaining length
  if (options.username.len > 0) {
    rem_len += options.username.len + 2;

    // add password if present to remaining length
    if (options.password.len > 0) {
      rem_len += options.password.len + 2;
    }
  }

  // check remaining length length
  int rem_len_len;
  lwmqtt_err_t err = lwmqtt_varnum_length(rem_len, &rem_len_len);
  if (err == LWMQTT_VARNUM_OVERFLOW) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // prepare header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_CONNECT_PACKET;

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write remaining length
  err = lwmqtt_write_varnum(&buf_ptr, buf_end, rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write version string
  err = lwmqtt_write_string(&buf_ptr, buf_end, lwmqtt_string("MQTT"));
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write version number
  err = lwmqtt_write_byte(&buf_ptr, buf_end, 4);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // prepare flags
  lwmqtt_connect_flags_t flags = {0};
  flags.bits.clean_session = options.clean_session ? 1 : 0;

  // set will flags if present
  if (will != NULL) {
    flags.bits.will = 1;
    flags.bits.will_qos = (unsigned int)will->qos;
    flags.bits.will_retain = will->retained ? 1 : 0;
  }

  // set username flag if present
  if (options.username.len > 0) {
    flags.bits.username = 1;

    // set password flag if present
    if (options.password.len > 0) {
      flags.bits.password = 1;
    }
  }

  // write flags
  err = lwmqtt_write_byte(&buf_ptr, buf_end, flags.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write keep alive
  err = lwmqtt_write_num(&buf_ptr, buf_end, options.keep_alive);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write client id
  err = lwmqtt_write_string(&buf_ptr, buf_end, options.client_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write will if present
  if (will != NULL) {
    // write topic
    err = lwmqtt_write_string(&buf_ptr, buf_end, will->topic);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // write payload length
    err = lwmqtt_write_num(&buf_ptr, buf_end, (uint16_t)will->payload.len);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // write payload
    err = lwmqtt_write_data(&buf_ptr, buf_end, (uint8_t *)will->payload.data, will->payload.len);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // write username if present
  if (flags.bits.username) {
    err = lwmqtt_write_string(&buf_ptr, buf_end, options.username);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // write password if present
  if (flags.bits.username && flags.bits.password) {
    err = lwmqtt_write_string(&buf_ptr, buf_end, options.password);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // set written length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_connack(uint8_t *buf, size_t buf_len, bool *session_present,
                                   lwmqtt_return_code_t *return_code) {
  // prepare pointers
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  lwmqtt_header_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (header.bits.type != LWMQTT_CONNACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf_end, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check remaining length
  if (rem_len != 2) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // read flags
  lwmqtt_connack_flags_t flags;
  err = lwmqtt_read_byte(&buf_ptr, buf_end, &flags.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read return code
  uint8_t raw_return_code;
  err = lwmqtt_read_byte(&buf_ptr, buf_end, &raw_return_code);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set variables
  *session_present = flags.bits.session_present == 1;
  *return_code = (lwmqtt_return_code_t)raw_return_code;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_zero(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_packet_type_t packet_type) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = packet_type;
  lwmqtt_err_t err = lwmqtt_write_byte(&buf_ptr, buf_end, header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write remaining length
  err = lwmqtt_write_varnum(&buf_ptr, buf_end, 0);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_ack(uint8_t *buf, size_t buf_len, lwmqtt_packet_type_t *packet_type, bool *dup,
                               uint16_t *packet_id) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  lwmqtt_header_t header = {0};
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set variables
  *dup = header.bits.dup == 1;
  *packet_type = (lwmqtt_packet_type_t)header.bits.type;

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf + buf_len, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check remaining length
  if (rem_len != 2) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // read packet id
  err = lwmqtt_read_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_ack(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_packet_type_t packet_type, bool dup,
                               uint16_t packet_id) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // prepare header
  lwmqtt_header_t header = {0};
  header.bits.type = packet_type;
  header.bits.dup = dup ? 1 : 0;
  header.bits.qos = (packet_type == LWMQTT_PUBREL_PACKET) ? 1 : 0;

  // write header
  lwmqtt_err_t err = lwmqtt_write_byte(&buf_ptr, buf_end, header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write remaining length
  err = lwmqtt_write_varnum(&buf_ptr, buf_end, 2);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write packet id
  err = lwmqtt_write_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set written length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_publish(uint8_t *buf, size_t buf_len, bool *dup, uint16_t *packet_id, lwmqtt_string_t *topic,
                                   lwmqtt_message_t *msg) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  lwmqtt_header_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (header.bits.type != LWMQTT_PUBLISH_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // set variables
  *dup = header.bits.dup == 1;
  msg->qos = (lwmqtt_qos_t)header.bits.qos;
  msg->retained = header.bits.retain == 1;

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf_end, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check remaining length (topic length)
  if (rem_len < 2) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // check buffer capacity
  if (buf_end - buf_ptr < rem_len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // reset buf end
  buf_end = buf_ptr + rem_len;

  // read topic
  err = lwmqtt_read_string(&buf_ptr, buf_end, topic);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read packet id if qos is at least 1
  if (msg->qos > 0) {
    err = lwmqtt_read_num(&buf_ptr, buf_end, packet_id);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  } else {
    *packet_id = 0;
  }

  // set payload length
  msg->payload_len = buf_end - buf_ptr;

  // read payload
  err = lwmqtt_read_data(&buf_ptr, buf_end, &msg->payload, buf_end - buf_ptr);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }


  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_publish(uint8_t *buf, size_t buf_len, size_t *len, bool dup, uint16_t packet_id,
                                   lwmqtt_string_t topic, lwmqtt_message_t msg) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // calculate remaining length
  uint32_t rem_len = 2 + topic.len + (uint32_t)msg.payload_len;
  if (msg.qos > 0) {
    rem_len += 2;
  }

  // check remaining length length
  int rem_len_len;
  lwmqtt_err_t err = lwmqtt_varnum_length(rem_len, &rem_len_len);
  if (err == LWMQTT_VARNUM_OVERFLOW) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // prepare header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_PUBLISH_PACKET;
  header.bits.dup = dup ? 1 : 0;
  header.bits.qos = (unsigned int)msg.qos;
  header.bits.retain = msg.retained ? 1 : 0;

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write remaining length
  err = lwmqtt_write_varnum(&buf_ptr, buf_end, rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write topic
  err = lwmqtt_write_string(&buf_ptr, buf_end, topic);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write packet id if qos is at least 1
  if (msg.qos > 0) {
    err = lwmqtt_write_num(&buf_ptr, buf_end, packet_id);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // write payload
  err = lwmqtt_write_data(&buf_ptr, buf_end, msg.payload, msg.payload_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_subscribe(uint8_t *buf, size_t buf_len, size_t *len, uint16_t packet_id, int count,
                                     lwmqtt_string_t *topic_filters, lwmqtt_qos_t *qos_levels) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // calculate remaining length
  uint32_t rem_len = 2;
  for (int i = 0; i < count; i++) {
    rem_len += 2 + topic_filters[i].len + 1;
  }

  // check remaining length length
  int rem_len_len;
  lwmqtt_err_t err = lwmqtt_varnum_length(rem_len, &rem_len_len);
  if (err == LWMQTT_VARNUM_OVERFLOW) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // prepare header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_SUBSCRIBE_PACKET;
  header.bits.qos = 1;

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write remaining length
  err = lwmqtt_write_varnum(&buf_ptr, buf_end, rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write packet id
  err = lwmqtt_write_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write all subscriptions
  for (int i = 0; i < count; i++) {
    // write topic
    err = lwmqtt_write_string(&buf_ptr, buf_end, topic_filters[i]);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // write qos level
    err = lwmqtt_write_byte(&buf_ptr, buf_end, (uint8_t)qos_levels[i]);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // set length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_suback(uint8_t *buf, size_t buf_len, uint16_t *packet_id, int max_count, int *count,
                                  lwmqtt_qos_t *granted_qos_levels) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  lwmqtt_header_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  } else if (header.bits.type != LWMQTT_SUBACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf_end, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check remaining length (packet id + min. one suback code)
  if (rem_len < 3) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // read packet id
  err = lwmqtt_read_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read all suback codes
  for (*count = 0; *count < (int)rem_len - 2; (*count)++) {
    // check max count
    if (*count > max_count) {
      return LWMQTT_SUBACK_ARRAY_OVERFLOW;
    }

    // read qos level
    uint8_t raw_qos_level;
    err = lwmqtt_read_byte(&buf_ptr, buf_end, &raw_qos_level);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    // set qos level
    granted_qos_levels[*count] = (lwmqtt_qos_t)raw_qos_level;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_unsubscribe(uint8_t *buf, size_t buf_len, size_t *len, uint16_t packet_id, int count,
                                       lwmqtt_string_t *topic_filters) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // calculate remaining length
  uint32_t rem_len = 2;
  for (int i = 0; i < count; i++) {
    rem_len += 2 + topic_filters[i].len;
  }

  // check remaining length length
  int rem_len_len;
  lwmqtt_err_t err = lwmqtt_varnum_length(rem_len, &rem_len_len);
  if (err == LWMQTT_VARNUM_OVERFLOW) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // prepare header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_UNSUBSCRIBE_PACKET;
  header.bits.qos = 1;

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header.byte);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write remaining length
  err = lwmqtt_write_varnum(&buf_ptr, buf_end, rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write packet id
  err = lwmqtt_write_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write topics
  for (int i = 0; i < count; i++) {
    err = lwmqtt_write_string(&buf_ptr, buf_end, topic_filters[i]);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // set length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}
