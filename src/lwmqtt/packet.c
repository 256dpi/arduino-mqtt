#include "packet.h"

static lwmqtt_err_t decode_props(uint8_t **buf, const uint8_t *buf_len, lwmqtt_protocol_t protocol,
                                 lwmqtt_serialized_properties_t *props);

lwmqtt_err_t lwmqtt_detect_packet_type(uint8_t *buf, size_t buf_len, lwmqtt_packet_type_t *packet_type) {
  // set default packet type
  *packet_type = LWMQTT_NO_PACKET;

  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // prepare header
  uint8_t header;

  // read header
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // get packet type
  *packet_type = (lwmqtt_packet_type_t)lwmqtt_read_bits(header, 4, 4);

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

lwmqtt_err_t lwmqtt_encode_connect(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_protocol_t protocol,
                                   lwmqtt_options_t options, lwmqtt_will_t *will) {
  // prepare pointers
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // fixed header is 10
  uint32_t rem_len = 10 + lwmqtt_propslen(protocol, options.properties);

  // add client id to remaining length
  rem_len += options.client_id.len + 2;

  // add will if present to remaining length
  if (will != NULL) {
    rem_len += will->topic.len + 2 + will->payload.len + 2;
    rem_len += lwmqtt_propslen(protocol, will->properties);
  }

  // add username if present to remaining length
  if (options.username.len > 0) {
    rem_len += options.username.len + 2;
  }

  // add password if present to remaining length
  if ((options.username.len > 0 || protocol == LWMQTT_MQTT5) && options.password.len > 0) {
    rem_len += options.password.len + 2;
  }

  // check remaining length length
  int rem_len_len;
  lwmqtt_err_t err = lwmqtt_varnum_length(rem_len, &rem_len_len);
  if (err == LWMQTT_VARNUM_OVERFLOW) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // prepare header
  uint8_t header = 0;
  lwmqtt_write_bits(&header, LWMQTT_CONNECT_PACKET, 4, 4);

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
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
  err = lwmqtt_write_byte(&buf_ptr, buf_end, protocol == LWMQTT_MQTT311 ? 4 : 5);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // prepare flags
  uint8_t flags = 0;

  // set clean session
  lwmqtt_write_bits(&flags, (uint8_t)(options.clean_session), 1, 1);

  // set will flags if present
  if (will != NULL) {
    lwmqtt_write_bits(&flags, 1, 2, 1);
    lwmqtt_write_bits(&flags, will->qos, 3, 2);
    lwmqtt_write_bits(&flags, (uint8_t)(will->retained), 5, 1);
  }

  // set username flag if present
  if (options.username.len > 0) {
    lwmqtt_write_bits(&flags, 1, 7, 1);
  }

  if ((options.username.len > 0 || protocol == LWMQTT_MQTT5) && options.password.len > 0) {
    lwmqtt_write_bits(&flags, 1, 6, 1);
  }

  // write flags
  err = lwmqtt_write_byte(&buf_ptr, buf_end, flags);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write keep alive
  err = lwmqtt_write_num(&buf_ptr, buf_end, options.keep_alive);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write connection properties
  err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, options.properties);
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
    err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, will->properties);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

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
  if (options.username.len > 0) {
    err = lwmqtt_write_string(&buf_ptr, buf_end, options.username);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // write password if present
  if ((options.username.len > 0 || protocol == LWMQTT_MQTT5) && options.password.len > 0) {
    err = lwmqtt_write_string(&buf_ptr, buf_end, options.password);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // set written length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_connack(uint8_t *buf, size_t buf_len, lwmqtt_protocol_t protocol, bool *session_present,
                                   lwmqtt_return_code_t *return_code) {
  // prepare pointers
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  uint8_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check packet type
  if (lwmqtt_read_bits(header, 4, 4) != LWMQTT_CONNACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf_end, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check remaining length
  if (protocol == LWMQTT_MQTT311 && rem_len != 2) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // read flags
  uint8_t flags;
  err = lwmqtt_read_byte(&buf_ptr, buf_end, &flags);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read return code
  uint8_t raw_return_code;
  err = lwmqtt_read_byte(&buf_ptr, buf_end, &raw_return_code);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // get session present
  *session_present = lwmqtt_read_bits(flags, 7, 1) == 1;

  // get return code
  switch (raw_return_code) {
    case 0:
      *return_code = LWMQTT_CONNECTION_ACCEPTED;
      break;
    case 1:
      *return_code = LWMQTT_UNACCEPTABLE_PROTOCOL;
      break;
    case 2:
      *return_code = LWMQTT_IDENTIFIER_REJECTED;
      break;
    case 3:
      *return_code = LWMQTT_SERVER_UNAVAILABLE;
      break;
    case 4:
      *return_code = LWMQTT_BAD_USERNAME_OR_PASSWORD;
      break;
    case 5:
      *return_code = LWMQTT_NOT_AUTHORIZED;
      break;
    default:
      *return_code = LWMQTT_UNKNOWN_RETURN_CODE;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_zero(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_packet_type_t packet_type) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // write header
  uint8_t header = 0;
  lwmqtt_write_bits(&header, packet_type, 4, 4);
  lwmqtt_err_t err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
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

lwmqtt_err_t lwmqtt_decode_ack(uint8_t *buf, size_t buf_len, lwmqtt_protocol_t protocol,
                               lwmqtt_packet_type_t packet_type, bool *dup, uint16_t *packet_id, uint8_t *status,
                               lwmqtt_serialized_properties_t *props) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  *status = 0;
  props->size = 0;

  // read header
  uint8_t header = 0;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check packet type
  if (lwmqtt_read_bits(header, 4, 4) != packet_type) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // get dup
  *dup = lwmqtt_read_bits(header, 3, 1) == 1;

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf + buf_len, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check remaining length
  if (protocol == LWMQTT_MQTT311 && rem_len != 2) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // read packet id
  err = lwmqtt_read_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  rem_len -= 2;

  if (rem_len > 0) {
    lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, status);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
    rem_len--;
  }

  if (rem_len > 0) {
    err = decode_props(&buf_ptr, buf_end, protocol, props);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  if (*status != 0) {
    return LWMQTT_PUBACK_NACKED;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_ack(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_protocol_t protocol,
                               lwmqtt_packet_type_t packet_type, bool dup, uint16_t packet_id, uint8_t status,
                               lwmqtt_properties_t props) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // prepare header
  uint8_t header = 0;

  // set packet type
  lwmqtt_write_bits(&header, packet_type, 4, 4);

  // set dup
  lwmqtt_write_bits(&header, (uint8_t)(dup), 3, 1);

  // set qos
  lwmqtt_write_bits(&header, (uint8_t)(packet_type == LWMQTT_PUBREL_PACKET ? LWMQTT_QOS1 : LWMQTT_QOS0), 1, 2);

  // write header
  lwmqtt_err_t err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  size_t rem_len = 2 + (protocol == LWMQTT_MQTT5 ? 1 : 0);
  if (props.len > 0) {
    rem_len += lwmqtt_propslen(protocol, props);
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

  if (protocol == LWMQTT_MQTT5) {
    err = lwmqtt_write_byte(&buf_ptr, buf_end, status);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  if (props.len > 0) {
    err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, props);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // set written length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t decode_props(uint8_t **buf, const uint8_t *buf_len, lwmqtt_protocol_t protocol,
                                 lwmqtt_serialized_properties_t *props) {
  if (protocol == LWMQTT_MQTT311) {
    return LWMQTT_SUCCESS;
  }
  uint32_t prop_len;
  lwmqtt_err_t err = lwmqtt_read_varnum(buf, buf_len, &prop_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }
  props->size = (size_t)prop_len;
  props->start = *buf;

  *buf += prop_len;
  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_publish(uint8_t *buf, size_t buf_len, lwmqtt_protocol_t protocol, bool *dup,
                                   uint16_t *packet_id, lwmqtt_string_t *topic, lwmqtt_message_t *msg,
                                   lwmqtt_serialized_properties_t *props) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  uint8_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check packet type
  if (lwmqtt_read_bits(header, 4, 4) != LWMQTT_PUBLISH_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // get dup
  *dup = lwmqtt_read_bits(header, 3, 1) == 1;

  // get retained
  msg->retained = lwmqtt_read_bits(header, 0, 1) == 1;

  // get qos
  switch (lwmqtt_read_bits(header, 1, 2)) {
    case 0:
      msg->qos = LWMQTT_QOS0;
      break;
    case 1:
      msg->qos = LWMQTT_QOS1;
      break;
    case 2:
      msg->qos = LWMQTT_QOS2;
      break;
    default:
      msg->qos = LWMQTT_QOS0;
      break;
  }

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
  if ((uint32_t)(buf_end - buf_ptr) < rem_len) {
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

  err = decode_props(&buf_ptr, buf_end, protocol, props);
  if (err != LWMQTT_SUCCESS) {
    return err;
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

lwmqtt_err_t lwmqtt_encode_publish(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_protocol_t protocol, bool dup,
                                   uint16_t packet_id, lwmqtt_string_t topic, lwmqtt_message_t msg,
                                   lwmqtt_properties_t props) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // calculate remaining length
  uint32_t rem_len = 2 + topic.len + (uint32_t)msg.payload_len + lwmqtt_propslen(protocol, props);
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
  uint8_t header = 0;

  // set packet type
  lwmqtt_write_bits(&header, LWMQTT_PUBLISH_PACKET, 4, 4);

  // set dup
  lwmqtt_write_bits(&header, (uint8_t)(dup), 3, 1);

  // set qos
  lwmqtt_write_bits(&header, msg.qos, 1, 2);

  // set retained
  lwmqtt_write_bits(&header, (uint8_t)(msg.retained), 0, 1);

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
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

  err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, props);
  if (err != LWMQTT_SUCCESS) {
    return err;
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

static uint8_t encode_sub_opt(lwmqtt_sub_options_t opt) {
  return (uint8_t)opt.qos | ((uint8_t)opt.retain_handling << 4) | (opt.retain_as_published ? 0x08 : 0) |
         (opt.no_local ? 0x04 : 0);
}

lwmqtt_err_t lwmqtt_encode_subscribe(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_protocol_t protocol,
                                     uint16_t packet_id, int count, lwmqtt_string_t *topic_filters,
                                     lwmqtt_sub_options_t *sub_options, lwmqtt_properties_t props) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // calculate remaining length
  uint32_t rem_len = 2 + lwmqtt_propslen(protocol, props);
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
  uint8_t header = 0;

  // set packet type
  lwmqtt_write_bits(&header, LWMQTT_SUBSCRIBE_PACKET, 4, 4);

  // set qos
  lwmqtt_write_bits(&header, LWMQTT_QOS1, 1, 2);

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
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

  err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, props);
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

    // write subscription options
    err = lwmqtt_write_byte(&buf_ptr, buf_end, encode_sub_opt(sub_options[i]));
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  // set length
  *len = buf_ptr - buf;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_suback(uint8_t *buf, size_t buf_len, uint16_t *packet_id, lwmqtt_protocol_t protocol,
                                  int max_count, int *count, lwmqtt_qos_t *granted_qos_levels) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  uint8_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check packet type
  if (lwmqtt_read_bits(header, 4, 4) != LWMQTT_SUBACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf_end, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }
  uint8_t *end = buf_ptr + rem_len;

  // check remaining length (packet id + min. one suback code)
  if (rem_len < 3) {
    return LWMQTT_REMAINING_LENGTH_MISMATCH;
  }

  // read packet id
  err = lwmqtt_read_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  lwmqtt_serialized_properties_t props;
  err = decode_props(&buf_ptr, buf_end, protocol, &props);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read all suback codes
  for (*count = 0; buf_ptr < end; (*count)++) {
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
    switch (raw_qos_level) {
      case 0:
        granted_qos_levels[*count] = LWMQTT_QOS0;
        break;
      case 1:
        granted_qos_levels[*count] = LWMQTT_QOS1;
        break;
      case 2:
        granted_qos_levels[*count] = LWMQTT_QOS2;
        break;
      default:
        granted_qos_levels[*count] = protocol == LWMQTT_MQTT311 ? 0x80 : (lwmqtt_qos_t)raw_qos_level;
        break;
    }
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_unsuback(uint8_t *buf, size_t buf_len, uint16_t *packet_id, lwmqtt_protocol_t protocol,
                                    int max_count, int *count, lwmqtt_unsubscribe_status_t *statuses) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // read header
  uint8_t header;
  lwmqtt_err_t err = lwmqtt_read_byte(&buf_ptr, buf_end, &header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // check packet type
  if (lwmqtt_read_bits(header, 4, 4) != LWMQTT_UNSUBACK_PACKET) {
    return LWMQTT_MISSING_OR_WRONG_PACKET;
  }

  // read remaining length
  uint32_t rem_len;
  err = lwmqtt_read_varnum(&buf_ptr, buf_end, &rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }
  uint8_t *end = buf_ptr + rem_len;

  // read packet id
  err = lwmqtt_read_num(&buf_ptr, buf_end, packet_id);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  if (protocol == LWMQTT_MQTT311) {
    for (int i = 0; i < max_count; i++) {
      statuses[i] = LWMQTT_UNSUB_SUCCESS;
    }
    *count = max_count;
    return LWMQTT_SUCCESS;
  }

  lwmqtt_serialized_properties_t props = {0, 0};
  err = decode_props(&buf_ptr, buf_end, protocol, &props);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read all suback codes
  for (*count = 0; buf_ptr < end; (*count)++) {
    // check max count
    if (*count > max_count) {
      return LWMQTT_SUBACK_ARRAY_OVERFLOW;
    }

    // read qos level
    uint8_t st;
    err = lwmqtt_read_byte(&buf_ptr, buf_end, &st);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
    statuses[*count] = (lwmqtt_unsubscribe_status_t)st;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_unsubscribe(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_protocol_t protocol,
                                       uint16_t packet_id, int count, lwmqtt_string_t *topic_filters,
                                       lwmqtt_properties_t props) {
  // prepare pointer
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf + buf_len;

  // calculate remaining length
  uint32_t rem_len = 2 + lwmqtt_propslen(protocol, props);
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
  uint8_t header = 0;

  // set packet type
  lwmqtt_write_bits(&header, LWMQTT_UNSUBSCRIBE_PACKET, 4, 4);

  // set qos
  lwmqtt_write_bits(&header, LWMQTT_QOS1, 1, 2);

  // write header
  err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
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

  err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, props);
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

lwmqtt_err_t lwmqtt_encode_disconnect(uint8_t *buf, size_t buf_len, size_t *len, lwmqtt_protocol_t protocol,
                                      uint8_t reason, lwmqtt_properties_t props) {
  uint8_t *buf_ptr = buf;
  uint8_t *buf_end = buf_ptr + buf_len;

  uint8_t header = 0;
  lwmqtt_write_bits(&header, LWMQTT_DISCONNECT_PACKET, 4, 4);
  lwmqtt_err_t err = lwmqtt_write_byte(&buf_ptr, buf_end, header);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  uint32_t rem_len = 0;
  if (protocol == LWMQTT_MQTT5) {
    rem_len = 1 + lwmqtt_propslen(protocol, props);
  }

  err = lwmqtt_write_varnum(&buf_ptr, buf_end, rem_len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  if (protocol == LWMQTT_MQTT5) {
    err = lwmqtt_write_byte(&buf_ptr, buf_end, reason);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }

    err = lwmqtt_write_props(&buf_ptr, buf_end, protocol, props);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  *len = buf_ptr - buf;
  return LWMQTT_SUCCESS;
}
