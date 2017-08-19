#include <string.h>

#include "lwmqtt.h"

lwmqtt_string_t lwmqtt_string(const char *str) {
  // get length
  uint16_t len = (uint16_t)strlen(str);

  return (lwmqtt_string_t){len, (char *)str};
}

int lwmqtt_strcmp(lwmqtt_string_t a, const char *b) {
  // get length of b
  size_t len = strlen(b);

  // otherwise check if length matches
  if (len != a.len) {
    return -1;
  }

  // compare memory
  return strncmp(a.data, b, len);
}
