#include <string.h>

#include "helpers.h"

lwmqtt_string_t lwmqtt_str(const char *str) { return (lwmqtt_string_t){(int)strlen(str), (char *)str}; }

int lwmqtt_strcmp(lwmqtt_string_t *a, char *b) {
  // get length of b
  int len = (int)strlen(b);

  // otherwise check if length matches
  if (len != a->len) {
    return -1;
  }

  // compare memory
  return strncmp(a->data, b, (size_t)len);
}

void lwmqtt_write_string(unsigned char **pptr, lwmqtt_string_t string) {
  // write length prefixed string if length is given
  if (string.len > 0) {
    lwmqtt_write_int(pptr, string.len);
    memcpy(*pptr, string.data, string.len);
    *pptr += string.len;
    return;
  }

  // write zero
  lwmqtt_write_int(pptr, 0);
}

bool lwmqtt_read_string(lwmqtt_string_t *str, unsigned char **pptr, unsigned char *end_ptr) {
  // check if at lest 2 bytes
  if (end_ptr - (*pptr) <= 1) {
    return false;
  }

  // read length
  int len = lwmqtt_read_int(pptr);

  // check if string end is overflowing the end pointer
  if (&(*pptr)[len] > end_ptr) {
    return false;
  }

  // set string
  str->len = len;
  str->data = (char *)*pptr;

  // advance pointer
  *pptr += str->len;

  return true;
}

int lwmqtt_read_int(unsigned char **pptr) {
  // get pointer
  unsigned char *ptr = *pptr;

  // read two byte integer
  int num = 256 * (*ptr) + (*(ptr + 1));

  // adjust pointer
  *pptr += 2;

  return num;
}

unsigned char lwmqtt_read_char(unsigned char **pptr) {
  // read single char
  unsigned char chr = **pptr;

  // adjust pointer
  (*pptr)++;

  return chr;
}

void lwmqtt_write_char(unsigned char **pptr, unsigned char chr) {
  // write single char
  **pptr = chr;

  // adjust pointer
  (*pptr)++;
}

void lwmqtt_write_int(unsigned char **pptr, int num) {
  // write first byte
  **pptr = (unsigned char)(num / 256);

  // adjust pointer
  (*pptr)++;

  // write second byte
  **pptr = (unsigned char)(num % 256);

  // adjust pointer
  (*pptr)++;
}
