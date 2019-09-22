#include <string.h>

#include "helpers.h"

uint8_t lwmqtt_read_bits(uint8_t byte, int pos, int num) { return (byte & (uint8_t)((~(0xFF << num)) << pos)) >> pos; }

void lwmqtt_write_bits(uint8_t *byte, uint8_t value, int pos, int num) {
  *byte = (*byte & ~(uint8_t)((~(0xFF << num)) << pos)) | (value << pos);
}

lwmqtt_err_t lwmqtt_read_data(uint8_t **buf, const uint8_t *buf_end, uint8_t **data, size_t len) {
  // check zero length
  if (len == 0) {
    *data = NULL;
    return LWMQTT_SUCCESS;
  }

  // check buffer size
  if ((size_t)(buf_end - (*buf)) < len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // read data
  *data = *buf;

  // advance pointer
  *buf += len;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_write_data(uint8_t **buf, const uint8_t *buf_end, uint8_t *data, size_t len) {
  // check zero length
  if (len == 0) {
    return LWMQTT_SUCCESS;
  }

  // check buffer size
  if ((size_t)(buf_end - (*buf)) < len) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write data
  memcpy(*buf, data, len);

  // advance pointer
  *buf += len;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_read_num(uint8_t **buf, const uint8_t *buf_end, uint16_t *num) {
  // check buffer size
  if ((size_t)(buf_end - (*buf)) < 2) {
    *num = 0;
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // read two byte integer
  *num = (uint16_t)256 * (*buf)[0] + (*buf)[1];

  // adjust pointer
  *buf += 2;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_write_num(uint8_t **buf, const uint8_t *buf_end, uint16_t num) {
  // check buffer size
  if ((size_t)(buf_end - (*buf)) < 2) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write bytes
  (*buf)[0] = (uint8_t)(num / 256);
  (*buf)[1] = (uint8_t)(num % 256);

  // adjust pointer
  *buf += 2;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_write_num32(uint8_t **buf, const uint8_t *buf_end, uint32_t num) {
  // check buffer size
  if ((size_t)(buf_end - (*buf)) < 4) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write bytes
  (*buf)[0] = (uint8_t)(num >> 24);
  (*buf)[1] = (uint8_t)((num >> 16) & 0xff);
  (*buf)[2] = (uint8_t)((num >> 8) & 0xff);
  (*buf)[3] = (uint8_t)(num & 0xff);

  // adjust pointer
  *buf += 4;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_read_num32(uint8_t **buf, const uint8_t *buf_end, uint32_t *num) {
  if ((size_t)(buf_end - (*buf)) < 4) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // read four byte integer
  *num = ((uint32_t)(*buf)[0] << 24) | ((uint32_t)(*buf)[1] << 16) | ((uint32_t)(*buf)[2] << 8) | (uint32_t)(*buf)[3];

  *buf += 4;
  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_read_string(uint8_t **buf, const uint8_t *buf_end, lwmqtt_string_t *str) {
  // read length
  uint16_t len;
  lwmqtt_err_t err = lwmqtt_read_num(buf, buf_end, &len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // read data
  err = lwmqtt_read_data(buf, buf_end, (uint8_t **)&str->data, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // set length
  str->len = len;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_write_string(uint8_t **buf, const uint8_t *buf_end, lwmqtt_string_t str) {
  // write string length
  lwmqtt_err_t err = lwmqtt_write_num(buf, buf_end, str.len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  // write data
  err = lwmqtt_write_data(buf, buf_end, (uint8_t *)str.data, str.len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_read_byte(uint8_t **buf, const uint8_t *buf_end, uint8_t *byte) {
  // check buffer size
  if ((size_t)(buf_end - (*buf)) < 1) {
    *byte = 0;
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // read byte
  *byte = (*buf)[0];

  // adjust pointer
  *buf += 1;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_write_byte(uint8_t **buf, const uint8_t *buf_end, uint8_t byte) {
  // check buffer size
  if ((size_t)(buf_end - (*buf)) < 1) {
    return LWMQTT_BUFFER_TOO_SHORT;
  }

  // write byte
  (*buf)[0] = byte;

  // adjust pointer
  *buf += 1;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_varnum_length(uint32_t varnum, int *len) {
  if (varnum < 128) {
    *len = 1;
    return LWMQTT_SUCCESS;
  } else if (varnum < 16384) {
    *len = 2;
    return LWMQTT_SUCCESS;
  } else if (varnum < 2097151) {
    *len = 3;
    return LWMQTT_SUCCESS;
  } else if (varnum < 268435455) {
    *len = 4;
    return LWMQTT_SUCCESS;
  } else {
    *len = 0;
    return LWMQTT_VARNUM_OVERFLOW;
  }
}

lwmqtt_err_t lwmqtt_read_varnum(uint8_t **buf, const uint8_t *buf_end, uint32_t *varnum) {
  // prepare last byte
  uint8_t byte;

  // prepare multiplier
  uint32_t multiplier = 1;

  // prepare length
  size_t len = 0;

  // initialize number
  *varnum = 0;

  // decode variadic number
  do {
    // increment length
    len++;

    // return error if buffer is to small
    if ((size_t)(buf_end - (*buf)) < len) {
      return LWMQTT_BUFFER_TOO_SHORT;
    }

    // return error if the length has overflowed
    if (len > 4) {
      return LWMQTT_VARNUM_OVERFLOW;
    }

    // read byte
    byte = (*buf)[len - 1];

    // add byte to number
    *varnum += (byte & 127) * multiplier;

    // increase multiplier
    multiplier *= 128;
  } while ((byte & 128) != 0);

  // adjust pointer
  *buf += len;

  return LWMQTT_SUCCESS;
}

lwmqtt_err_t lwmqtt_write_varnum(uint8_t **buf, const uint8_t *buf_end, uint32_t varnum) {
  // init len counter
  size_t len = 0;

  // encode variadic number
  do {
    // check overflow
    if (len == 4) {
      return LWMQTT_VARNUM_OVERFLOW;
    }

    // return error if buffer is to small
    if ((size_t)(buf_end - (*buf)) < len + 1) {
      return LWMQTT_BUFFER_TOO_SHORT;
    }

    // calculate current byte
    uint8_t byte = (uint8_t)(varnum % 128);

    // change remaining length
    varnum /= 128;

    // set the top bit of this byte if there are more to encode
    if (varnum > 0) {
      byte |= 0x80;
    }

    // write byte
    (*buf)[len++] = byte;
  } while (varnum > 0);

  // adjust pointer
  *buf += len;

  return LWMQTT_SUCCESS;
}

static lwmqtt_err_t write_prop(uint8_t **buf, const uint8_t *buf_end, lwmqtt_property_t prop) {
  lwmqtt_err_t err = lwmqtt_write_byte(buf, buf_end, prop.prop);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  switch (prop.prop) {
    // one byte
    case LWMQTT_PROP_PAYLOAD_FORMAT_INDICATOR:
    case LWMQTT_PROP_REQUEST_PROBLEM_INFORMATION:
    case LWMQTT_PROP_MAXIMUM_QOS:
    case LWMQTT_PROP_RETAIN_AVAILABLE:
    case LWMQTT_PROP_REQUEST_RESPONSE_INFORMATION:
    case LWMQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE:
    case LWMQTT_PROP_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
    case LWMQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE:
      return lwmqtt_write_byte(buf, buf_end, prop.value.byte);

    // two byte int
    case LWMQTT_PROP_SERVER_KEEP_ALIVE:
    case LWMQTT_PROP_RECEIVE_MAXIMUM:
    case LWMQTT_PROP_TOPIC_ALIAS_MAXIMUM:
    case LWMQTT_PROP_TOPIC_ALIAS:
      return lwmqtt_write_num(buf, buf_end, prop.value.int16);

    // 4 byte int
    case LWMQTT_PROP_MESSAGE_EXPIRY_INTERVAL:
    case LWMQTT_PROP_SESSION_EXPIRY_INTERVAL:
    case LWMQTT_PROP_WILL_DELAY_INTERVAL:
    case LWMQTT_PROP_MAXIMUM_PACKET_SIZE:
      return lwmqtt_write_num32(buf, buf_end, prop.value.int32);

    // Variable byte int
    case LWMQTT_PROP_SUBSCRIPTION_IDENTIFIER:
      return lwmqtt_write_varnum(buf, buf_end, prop.value.int32);

    // UTF-8 string
    case LWMQTT_PROP_CONTENT_TYPE:
    case LWMQTT_PROP_RESPONSE_TOPIC:
    case LWMQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER:
    case LWMQTT_PROP_AUTHENTICATION_METHOD:
    case LWMQTT_PROP_RESPONSE_INFORMATION:
    case LWMQTT_PROP_SERVER_REFERENCE:
    case LWMQTT_PROP_REASON_STRING:

    // Arbitrary blobs as the same encoding.
    case LWMQTT_PROP_CORRELATION_DATA:
    case LWMQTT_PROP_AUTHENTICATION_DATA:
      return lwmqtt_write_string(buf, buf_end, prop.value.str);

    case LWMQTT_PROP_USER_PROPERTY:
      lwmqtt_write_string(buf, buf_end, prop.value.pair.k);
      lwmqtt_write_string(buf, buf_end, prop.value.pair.v);
  }

  return LWMQTT_SUCCESS;
}

static size_t proplen(lwmqtt_property_t prop) {
  int ll;
  switch (prop.prop) {
    // one byte
    case LWMQTT_PROP_PAYLOAD_FORMAT_INDICATOR:
    case LWMQTT_PROP_REQUEST_PROBLEM_INFORMATION:
    case LWMQTT_PROP_MAXIMUM_QOS:
    case LWMQTT_PROP_RETAIN_AVAILABLE:
    case LWMQTT_PROP_REQUEST_RESPONSE_INFORMATION:
    case LWMQTT_PROP_WILDCARD_SUBSCRIPTION_AVAILABLE:
    case LWMQTT_PROP_SUBSCRIPTION_IDENTIFIER_AVAILABLE:
    case LWMQTT_PROP_SHARED_SUBSCRIPTION_AVAILABLE:
      return 2;

    // two byte int
    case LWMQTT_PROP_SERVER_KEEP_ALIVE:
    case LWMQTT_PROP_RECEIVE_MAXIMUM:
    case LWMQTT_PROP_TOPIC_ALIAS_MAXIMUM:
    case LWMQTT_PROP_TOPIC_ALIAS:
      return 3;

    // 4 byte int
    case LWMQTT_PROP_MESSAGE_EXPIRY_INTERVAL:
    case LWMQTT_PROP_SESSION_EXPIRY_INTERVAL:
    case LWMQTT_PROP_WILL_DELAY_INTERVAL:
    case LWMQTT_PROP_MAXIMUM_PACKET_SIZE:
      return 5;

    // Variable byte int
    case LWMQTT_PROP_SUBSCRIPTION_IDENTIFIER:
      lwmqtt_varnum_length(prop.value.int32, &ll);
      return 1 + ll;

    // UTF-8 string
    case LWMQTT_PROP_CONTENT_TYPE:
    case LWMQTT_PROP_RESPONSE_TOPIC:
    case LWMQTT_PROP_ASSIGNED_CLIENT_IDENTIFIER:
    case LWMQTT_PROP_AUTHENTICATION_METHOD:
    case LWMQTT_PROP_RESPONSE_INFORMATION:
    case LWMQTT_PROP_SERVER_REFERENCE:
    case LWMQTT_PROP_REASON_STRING:

    // Arbitrary blobs are the same encoding.
    case LWMQTT_PROP_CORRELATION_DATA:
    case LWMQTT_PROP_AUTHENTICATION_DATA:
      return 3 + prop.value.str.len;

    case LWMQTT_PROP_USER_PROPERTY:
      return 1 + 2 + prop.value.pair.k.len + 2 + prop.value.pair.v.len;
  }
  return 0;
}

// Length of the properties, not including their length.
static size_t propsintlen(lwmqtt_properties_t props) {
  uint32_t l = 0;

  for (int i = 0; i < props.len; i++) {
    l += proplen(props.props[i]);
  }

  return l;
}

// Length of a properties set as it may appear on the wire (including
// the length of the length).
size_t lwmqtt_propslen(lwmqtt_protocol_t prot, lwmqtt_properties_t props) {
  if (prot == LWMQTT_MQTT311) {
    return 0;
  }

  uint32_t l = propsintlen(props);
  int ll;
  // lwmqtt_err_t err =
  lwmqtt_varnum_length(l, &ll);

  return l + ll;
}

lwmqtt_err_t lwmqtt_write_props(uint8_t **buf, const uint8_t *buf_end, lwmqtt_protocol_t prot,
                                lwmqtt_properties_t props) {
  if (prot == LWMQTT_MQTT311) {
    return LWMQTT_SUCCESS;
  }

  size_t len = propsintlen(props);
  lwmqtt_err_t err = lwmqtt_write_varnum(buf, buf_end, len);
  if (err != LWMQTT_SUCCESS) {
    return err;
  }

  for (int i = 0; i < props.len; i++) {
    err = write_prop(buf, buf_end, props.props[i]);
    if (err != LWMQTT_SUCCESS) {
      return err;
    }
  }

  return LWMQTT_SUCCESS;
}
