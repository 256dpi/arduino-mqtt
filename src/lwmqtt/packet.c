#include "lwmqtt.h"
#include <string.h>

#include "packet.h"

typedef union {
  unsigned char byte;
  struct {
    unsigned int retain : 1;
    unsigned int qos : 2;
    unsigned int dup : 1;
    unsigned int type : 4;
  } bits;
} lwmqtt_header_t;

typedef union {
  unsigned char byte;
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
  unsigned char byte;
  struct {
    unsigned int _ : 7;
    unsigned int session_present : 1;
  } bits;
} lwmqtt_connack_flags_t;

lwmqtt_err_t lwmqtt_detect_packet_type(void *buf, lwmqtt_packet_type_t *packet_type) {
  // prepare pointer
  void *ptr = buf;

  // read header
  lwmqtt_header_t header;
  header.byte = lwmqtt_read_byte(&ptr);

  // set default packet type
  *packet_type = LWMQTT_NO_PACKET;

  // check if packet type is correct and can be received
  switch ((lwmqtt_packet_type_t)header.bits.type) {
    case LWMQTT_CONNACK_PACKET:
    case LWMQTT_PUBLISH_PACKET:
    case LWMQTT_PUBACK_PACKET:
    case LWMQTT_PUBREC_PACKET:
    case LWMQTT_PUBREL_PACKET:
    case LWMQTT_PUBCOMP_PACKET:
    case LWMQTT_SUBACK_PACKET:
    case LWMQTT_UNSUBACK_PACKET:
    case LWMQTT_PINGRESP_PACKET:
      *packet_type = (lwmqtt_packet_type_t)header.bits.type;
      return LWMQTT_SUCCESS;
    default:
      return LWMQTT_DECODE_ERROR;
  }
}

lwmqtt_err_t lwmqtt_detect_remaining_length(void *buf, int buf_len, long *rem_len) {
  // prepare pointer
  void *ptr = buf;

  // attempt to decode remaining length
  *rem_len = lwmqtt_read_varnum(&ptr, buf_len);
  if (*rem_len == -1) {
    *rem_len = 0;
    return LWMQTT_BUFFER_TOO_SHORT;
  } else if (*rem_len == -2) {
    *rem_len = 0;
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_connect(void *buf, int buf_len, int *len, lwmqtt_options_t *options, lwmqtt_will_t *will) {
  // prepare pointer
  void *ptr = buf;

  // fixed header is 10
  int rem_len = 10;

  // add client id to remaining length
  rem_len += options->client_id.len + 2;

  // add will if present to remaining length
  if (will != NULL) {
    rem_len += will->topic.len + 2 + will->message.payload_len + 2;
  }

  // add username if present to remaining length
  if (options->username.len > 0) {
    rem_len += options->username.len + 2;

    // add password if present to remaining length
    if (options->password.len > 0) {
      rem_len += options->password.len + 2;
    }
  }

  // calculate remaining length length
  int rem_len_len = lwmqtt_varnum_length(rem_len);
  if (rem_len_len < 0) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check buffer capacity
  if (1 + rem_len_len + rem_len > buf_len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_CONNECT_PACKET;
  lwmqtt_write_byte(&ptr, header.byte);

  // write remaining length
  lwmqtt_write_varnum(&ptr, rem_len);

  // write version
  lwmqtt_write_string(&ptr, lwmqtt_str("MQTT"));
  lwmqtt_write_byte(&ptr, 4);

  // prepare flags
  lwmqtt_connect_flags_t flags = {0};
  flags.bits.clean_session = options->clean_session ? 1 : 0;

  // set will flags if present
  if (will != NULL) {
    flags.bits.will = 1;
    flags.bits.will_qos = (unsigned int)will->message.qos;
    flags.bits.will_retain = will->message.retained ? 1 : 0;
  }

  // set username flag if present
  if (options->username.len > 0) {
    flags.bits.username = 1;

    // set password flag if present
    if (options->password.len > 0) {
      flags.bits.password = 1;
    }
  }

  // write flags
  lwmqtt_write_byte(&ptr, flags.byte);

  // write keep alive
  lwmqtt_write_num(&ptr, options->keep_alive);

  // write client id
  lwmqtt_write_string(&ptr, options->client_id);

  // write will topic and payload if present
  if (will != NULL) {
    lwmqtt_write_string(&ptr, will->topic);
    lwmqtt_write_num(&ptr, will->message.payload_len);
    memcpy(ptr, will->message.payload, will->message.payload_len);
    ptr += will->message.payload_len;
  }

  // write username if present
  if (flags.bits.username) {
    lwmqtt_write_string(&ptr, options->username);

    // write password if present
    if (flags.bits.password) {
      lwmqtt_write_string(&ptr, options->password);
    }
  }

  // set written length
  *len = (int)(ptr - buf);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_connack(void *buf, int buf_len, bool *session_present, lwmqtt_return_code_t *return_code) {
  // prepare pointer
  void *ptr = buf;

  // read header
  lwmqtt_header_t header;
  header.byte = lwmqtt_read_byte(&ptr);
  if (header.bits.type != LWMQTT_CONNACK_PACKET) {
    return LWMQTT_DECODE_ERROR;
  }

  // read remaining length
  long rem_len = lwmqtt_read_varnum(&ptr, buf_len - 1);
  if (rem_len == -1) {
    return LWMQTT_BUFFER_TOO_SHORT;
  } else if (rem_len == -2) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check remaining length and buffer size
  if (rem_len != 2 || buf_len < rem_len + 2) {
    return LWMQTT_LENGTH_MISMATCH;
  }

  // read flags
  lwmqtt_connack_flags_t flags;
  flags.byte = lwmqtt_read_byte(&ptr);
  *session_present = flags.bits.session_present == 1;
  *return_code = (lwmqtt_return_code_t)lwmqtt_read_byte(&ptr);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_zero(void *buf, int buf_len, int *len, lwmqtt_packet_type_t packet_type) {
  // prepare pointer
  void *ptr = buf;

  // check buffer capacity
  if (buf_len < 2) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = packet_type;
  lwmqtt_write_byte(&ptr, header.byte);

  // write remaining length
  lwmqtt_write_varnum(&ptr, 0);

  // set length
  *len = (int)(ptr - buf);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_ack(void *buf, int buf_len, lwmqtt_packet_type_t *packet_type, bool *dup, long *packet_id) {
  // prepare pointer
  void *ptr = buf;

  // read header
  lwmqtt_header_t header = {0};
  header.byte = lwmqtt_read_byte(&ptr);
  *dup = header.bits.dup == 1;
  *packet_type = (lwmqtt_packet_type_t)header.bits.type;

  // read remaining length
  long rem_len = lwmqtt_read_varnum(&ptr, buf_len - 1);
  if (rem_len == -1) {
    return LWMQTT_BUFFER_TOO_SHORT;
  } else if (rem_len == -2) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check remaining length and buffer size
  if (rem_len != 2 || buf_len < rem_len + 2) {
    return LWMQTT_LENGTH_MISMATCH;
  }

  // read packet id
  *packet_id = lwmqtt_read_num(&ptr);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_ack(void *buf, int buf_len, int *len, lwmqtt_packet_type_t packet_type, bool dup,
                               long packet_id) {
  // prepare pointer
  void *ptr = buf;

  // check buffer capacity
  if (buf_len < 4) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = packet_type;
  header.bits.dup = dup ? 1 : 0;
  header.bits.qos = (packet_type == LWMQTT_PUBREL_PACKET) ? 1 : 0;
  lwmqtt_write_byte(&ptr, header.byte);

  // write remaining length
  lwmqtt_write_varnum(&ptr, 2);

  // write packet id
  lwmqtt_write_num(&ptr, packet_id);

  // set written length
  *len = (int)(ptr - buf);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_publish(void *buf, int buf_len, bool *dup, lwmqtt_qos_t *qos, bool *retained,
                                   long *packet_id, lwmqtt_string_t *topic, void **payload, int *payload_len) {
  // prepare pointer
  void *ptr = buf;

  // read header
  lwmqtt_header_t header;
  header.byte = lwmqtt_read_byte(&ptr);
  if (header.bits.type != LWMQTT_PUBLISH_PACKET) {
    return LWMQTT_DECODE_ERROR;
  }

  // set dup
  *dup = header.bits.dup == 1;

  // set qos
  *qos = (lwmqtt_qos_t)header.bits.qos;

  // set retained
  *retained = header.bits.retain == 1;

  // read remaining length
  long rem_len = lwmqtt_read_varnum(&ptr, buf_len - 1);
  if (rem_len == -1) {
    return LWMQTT_BUFFER_TOO_SHORT;
  } else if (rem_len == -2) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check remaining length (topic length)
  if (rem_len < 2) {
    return LWMQTT_LENGTH_MISMATCH;
  }

  // check buffer size
  if (buf_len < 1 + lwmqtt_varnum_length(rem_len) + rem_len) {
    return LWMQTT_LENGTH_MISMATCH;
  }

  // calculate end pointer
  void *end_ptr = ptr + rem_len;

  // read topic
  long ret = lwmqtt_read_string(topic, &ptr, end_ptr);
  if (ret == -1) {
    return LWMQTT_BUFFER_TOO_SHORT;
  } else if (ret == -2) {
    return LWMQTT_DECODE_ERROR;
  }

  // read packet id if qos is at least 1
  if (*qos > 0) {
    // check buffer size
    if (end_ptr - ptr < 2) {
      return LWMQTT_BUFFER_TOO_SHORT;
    }

    *packet_id = lwmqtt_read_num(&ptr);
  } else {
    *packet_id = 0;
  }

  // set payload
  *payload_len = (int)(end_ptr - ptr);
  *payload = ptr;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_publish(void *buf, int buf_len, int *len, bool dup, lwmqtt_qos_t qos, bool retained,
                                   long packet_id, lwmqtt_string_t topic, void *payload, int payload_len) {
  // prepare pointer
  void *ptr = buf;

  // calculate remaining length
  long rem_len = 2 + topic.len + payload_len;
  if (qos > 0) {
    rem_len += 2;
  }

  // calculate remaining length length
  int rem_len_len = lwmqtt_varnum_length(rem_len);
  if (rem_len_len < 0) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check buffer capacity
  if (1 + rem_len_len + rem_len > buf_len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_PUBLISH_PACKET;
  header.bits.dup = dup ? 1 : 0;
  header.bits.qos = (unsigned int)qos;
  header.bits.retain = retained ? 1 : 0;
  lwmqtt_write_byte(&ptr, header.byte);

  // write remaining length
  lwmqtt_write_varnum(&ptr, rem_len);

  // write topic
  lwmqtt_write_string(&ptr, topic);

  // write packet id if qos is at least 1
  if (qos > 0) {
    lwmqtt_write_num(&ptr, packet_id);
  }

  // write payload
  memcpy(ptr, payload, payload_len);
  ptr += payload_len;

  // set length
  *len = (int)(ptr - buf);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_subscribe(void *buf, int buf_len, int *len, long packet_id, int count,
                                     lwmqtt_string_t *topic_filters, lwmqtt_qos_t *qos_levels) {
  // prepare pointer
  void *ptr = buf;

  // calculate remaining length
  int rem_len = 2;
  for (int i = 0; i < count; i++) {
    rem_len += 2 + topic_filters[i].len + 1;
  }

  // calculate remaining length length
  int rem_len_len = lwmqtt_varnum_length(rem_len);
  if (rem_len_len < 0) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check buffer capacity
  if (1 + rem_len_len + rem_len > buf_len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_SUBSCRIBE_PACKET;
  header.bits.qos = 1;
  lwmqtt_write_byte(&ptr, header.byte);

  // write remaining length
  lwmqtt_write_varnum(&ptr, rem_len);

  // write packet id
  lwmqtt_write_num(&ptr, packet_id);

  // write all topics
  for (int i = 0; i < count; i++) {
    lwmqtt_write_string(&ptr, topic_filters[i]);
    lwmqtt_write_byte(&ptr, (unsigned char)qos_levels[i]);
  }

  // set length
  *len = (int)(ptr - buf);

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_decode_suback(void *buf, int buf_len, long *packet_id, int max_count, int *count,
                                  lwmqtt_qos_t *granted_qos_levels) {
  // prepare pointer
  void *ptr = buf;

  // read header
  lwmqtt_header_t header;
  header.byte = lwmqtt_read_byte(&ptr);
  if (header.bits.type != LWMQTT_SUBACK_PACKET) {
    return LWMQTT_DECODE_ERROR;
  }

  // read remaining length
  long rem_len = lwmqtt_read_varnum(&ptr, buf_len - 1);
  if (rem_len == -1) {
    return LWMQTT_BUFFER_TOO_SHORT;
  } else if (rem_len == -2) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check remaining length (packet id + min. one suback code)
  if (rem_len < 3) {
    return LWMQTT_LENGTH_MISMATCH;
  }

  // check buffer size
  if (buf_len < 1 + lwmqtt_varnum_length(rem_len) + rem_len) {
    return LWMQTT_LENGTH_MISMATCH;
  }

  // read packet id
  *packet_id = lwmqtt_read_num(&ptr);

  // read all suback codes
  for (*count = 0; *count < rem_len - 2; (*count)++) {
    // check max count
    if (*count > max_count) {
      return LWMQTT_DECODE_ERROR;
    }

    // add qos level
    granted_qos_levels[*count] = (lwmqtt_qos_t)lwmqtt_read_byte(&ptr);
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_encode_unsubscribe(void *buf, int buf_len, int *len, long packet_id, int count,
                                       lwmqtt_string_t *topic_filters) {
  // prepare pointer
  void *ptr = buf;

  // calculate remaining length
  int rem_len = 2;
  for (int i = 0; i < count; i++) {
    rem_len += 2 + topic_filters[i].len;
  }

  // calculate remaining length length
  int rem_len_len = lwmqtt_varnum_length(rem_len);
  if (rem_len_len < 0) {
    return LWMQTT_REMAINING_LENGTH_OVERFLOW;
  }

  // check buffer capacity
  if (1 + rem_len_len + rem_len > buf_len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write header
  lwmqtt_header_t header = {0};
  header.bits.type = LWMQTT_UNSUBSCRIBE_PACKET;
  header.bits.qos = 1;
  lwmqtt_write_byte(&ptr, header.byte);

  // write remaining length
  lwmqtt_write_varnum(&ptr, rem_len);

  // write packet id
  lwmqtt_write_num(&ptr, packet_id);

  // write topics
  for (int i = 0; i < count; i++) {
    lwmqtt_write_string(&ptr, topic_filters[i]);
  }

  // set length
  *len = (int)(ptr - buf);

  return LWMQTT_SUCCESS;
}
