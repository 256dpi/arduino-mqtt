#ifndef LWMQTT_HELPERS_H
#define LWMQTT_HELPERS_H

#include <stdbool.h>

#include "lwmqtt.h"

/**
 * Reads a string from the specified buffer into the passed object. The pointer is incremented by the bytes read.
 *
 * @param str - The object into which the data is to be read.
 * @param buf - Pointer to the buffer.
 * @param buf_end - Pointer to the end of the buffer.
 * @return Length if successful, -1 if buffer is to short and -2 if overflowed.
 */
long lwmqtt_read_string(lwmqtt_string_t *str, void **buf, void *buf_end);

/**
 * Writes a string to the specified buffer. The pointer is incremented by the bytes written.
 *
 * @param buf - Pointer to the buffer.
 * @param string - The string to write.
 */
void lwmqtt_write_string(void **buf, lwmqtt_string_t string);

/**
 * Reads two byte number from the specified buffer. The pointer is incremented by two.
 *
 * @param buf - Pointer to the buffer.
 * @return The read number.
 */
long lwmqtt_read_num(void **buf);

/**
 * Writes a two byte number to the specified buffer. The pointer is incremented by two.
 *
 * @param buf - Pointer to the buffer.
 * @param num - The number to write.
 */
void lwmqtt_write_num(void **buf, long num);

/**
 * Reads one byte from the buffer. The pointer is incremented by one.
 *
 * @param buf - Pointer to the buffer.
 * @return The read byte.
 */
unsigned char lwmqtt_read_byte(void **buf);

/**
 * Writes one byte to the specified buffer. The pointer is incremented by one.
 *
 * @param buf - Pointer to the buffer.
 * @param byte - The byte to write.
 */
void lwmqtt_write_byte(void **buf, unsigned char byte);

/**
 * Returns the amount of bytes required by the variable number.
 *
 * @param num - The number to check.
 * @return The required length or -1 if overflowed.
 */
int lwmqtt_varnum_length(long num);

/**
 * Reads a variable number from the specified buffer. The pointer is incremented by the bytes read.
 *
 * @param buf - Pointer to the buffer.
 * @param buf_len - The length of the buffer.
 * @return Length if successful, -1 if buffer is to short and -2 if overflowed.
 */
long lwmqtt_read_varnum(void **buf, int buf_len);

/**
 * Writes a variable number to the specified buffer. The pointer is incremented by the bytes written.
 * The length required by the variable number needs to be checked beforehand using lwmqtt_varnum_length().
 *
 * @param buf - Pointer to the buffer.
 * @param num - The number to write.
 */
void lwmqtt_write_varnum(void **buf, long num);

#endif
