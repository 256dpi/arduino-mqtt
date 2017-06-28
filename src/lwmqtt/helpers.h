#ifndef LWMQTT_HELPERS_H
#define LWMQTT_HELPERS_H

#include <stdbool.h>

#include "lwmqtt.h"

/**
 * Reads a string object from the buffer and populates the passed object.
 *
 * @param str - The object into which the data is to be read.
 * @param pptr - Pointer to the output buffer - incremented by the number of bytes used & returned.
 * @param end_ptr - Pointer to the end of the data: do not read beyond.
 * @return One if successful, zero if not.
 */
bool lwmqtt_read_string(lwmqtt_string_t *str, unsigned char **pptr, unsigned char *end_ptr);

/**
 * Writes a string to an output buffer.
 *
 * @param pptr - Pointer to the output buffer - incremented by the number of bytes used & returned.
 * @param The string to write.
 */
void lwmqtt_write_string(unsigned char **pptr, lwmqtt_string_t string);

/**
 * Calculates an integer from two bytes read from the input buffer.
 *
 * @param pptr - Pointer to the input buffer - incremented by the number of bytes used & returned.
 * @return The integer value calculated.
 */
int lwmqtt_read_int(unsigned char **pptr);

/**
 * Reads one character from the input buffer.
 *
 * @param pptr - Pointer to the input buffer - incremented by the number of bytes used & returned.
 * @return The character read.
 */
unsigned char lwmqtt_read_char(unsigned char **pptr);

/**
 * Writes one character to an output buffer.
 *
 * @param pptr - Pointer to the output buffer - incremented by the number of bytes used & returned.
 * @param The character to write
 */
void lwmqtt_write_char(unsigned char **pptr, unsigned char chr);

/**
 * Writes an integer as 2 bytes to an output buffer.
 *
 * @param pptr - Pointer to the output buffer - incremented by the number of bytes used & returned.
 * @param The integer to write.
 */
void lwmqtt_write_int(unsigned char **pptr, int num);

#endif
