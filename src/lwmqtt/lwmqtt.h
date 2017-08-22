#ifndef LWMQTT_H
#define LWMQTT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * The error type used by all exposed APIs.
 */
typedef enum {
  LWMQTT_SUCCESS = 0,
  LWMQTT_BUFFER_TOO_SHORT = -1,
  LWMQTT_VARNUM_OVERFLOW = -2,
  LWMQTT_NETWORK_FAILED_CONNECT = -3,
  LWMQTT_NETWORK_TIMEOUT = -4,
  LWMQTT_NETWORK_FAILED_READ = -5,
  LWMQTT_NETWORK_FAILED_WRITE = -6,
  LWMQTT_REMAINING_LENGTH_OVERFLOW = -7,
  LWMQTT_REMAINING_LENGTH_MISMATCH = -8,
  LWMQTT_MISSING_OR_WRONG_PACKET = -9,
  LWMQTT_CONNECTION_DENIED = -10,
  LWMQTT_FAILED_SUBSCRIPTION = -11,
  LWMQTT_SUBACK_ARRAY_OVERFLOW = -12,
  LWMQTT_UNANSWERED_PING = -13,
} lwmqtt_err_t;

/**
 * A common string object.
 */
typedef struct {
  uint16_t len;
  char *data;
} lwmqtt_string_t;

/**
 * The initializer for string objects.
 */
#define lwmqtt_default_string \
  { 0, NULL }

/**
 * Returns a string object for the passed C string.
 *
 * @param str - The C string.
 * @return A string object.
 */
lwmqtt_string_t lwmqtt_string(const char *str);

/**
 * Compares a string object to a C string.
 *
 * @param a - The string object to compare.
 * @param b - The C string to compare.
 * @return Similarity e.g. strcmp().
 */
int lwmqtt_strcmp(lwmqtt_string_t a, const char *b);

/**
 * The available QOS levels.
 */
typedef enum { LWMQTT_QOS0 = 0, LWMQTT_QOS1 = 1, LWMQTT_QOS2 = 2, LWMQTT_QOS_FAILURE = 128 } lwmqtt_qos_t;

/**
 * The message object used to publish and receive messages.
 */
typedef struct {
  lwmqtt_qos_t qos;
  bool retained;
  uint8_t *payload;
  size_t payload_len;
} lwmqtt_message_t;

/**
 * The initializer for message objects.
 */
#define lwmqtt_default_message \
  { LWMQTT_QOS0, false, NULL, 0 }

/**
 * Forward declaration of the client object.
 */
typedef struct lwmqtt_client_t lwmqtt_client_t;

/**
 * The callback used to read from a network object.
 *
 * The callbacks is expected to read up to the amount of bytes in to the passed buffer. It should block the specified
 * timeout and wait for more incoming data.
 */
typedef lwmqtt_err_t (*lwmqtt_network_read_t)(lwmqtt_client_t *client, void *ref, uint8_t *buf, size_t len,
                                              size_t *read, int timeout);

/**
 * The callback used to write to a network object.
 *
 * The callback is expected to write up to the amount of bytes from the passed buffer. It should wait up to the
 * specified timeout to write the specified data to the network.
 */
typedef lwmqtt_err_t (*lwmqtt_network_write_t)(lwmqtt_client_t *client, void *ref, uint8_t *buf, size_t len,
                                               size_t *sent, int timeout);

/**
 * The callback used to set a timer.
 */
typedef void (*lwmqtt_timer_set_t)(lwmqtt_client_t *client, void *ref, int timeout);

/**
 * The callback used to get a timers value.
 */
typedef int (*lwmqtt_timer_get_t)(lwmqtt_client_t *client, void *ref);

/**
 * The callback used to forward incoming messages.
 *
 * Note: The callback is mostly executed because of a call to lwmqtt_yield() that processes incoming messages. However,
 * it is possible that the callback is also executed during a call to lwmqtt_subscribe(), lwmqtt_publish() or
 * lwmqtt_unsubscribe() if incoming messages are received between the required acknowledgements. It is therefore not
 * recommended to call any further lwmqtt methods in the callback as this might result in weird call stacks. The
 * callback should place the received messages in a queue and dispatch them after the caller has returned.
 */
typedef void (*lwmqtt_callback_t)(lwmqtt_client_t *client, void *ref, lwmqtt_string_t str, lwmqtt_message_t msg);

/**
 * The client object.
 */
struct lwmqtt_client_t {
  uint16_t last_packet_id;
  int keep_alive_interval;
  bool ping_outstanding;

  size_t write_buf_size, read_buf_size;
  uint8_t *write_buf, *read_buf;

  void *callback_ref;
  lwmqtt_callback_t callback;

  void *network;
  lwmqtt_network_read_t network_read;
  lwmqtt_network_write_t network_write;

  void *keep_alive_timer;
  void *command_timer;
  lwmqtt_timer_set_t timer_set;
  lwmqtt_timer_get_t timer_get;
};

/**
 * Will initialize the specified client object.
 *
 * @param client - The client object.
 * @param write_buf - The write buffer.
 * @param write_buf_size - The write buffer size.
 * @param read_buf - The read buffer.
 * @param read_buf_size - The read buffer size.
 */
void lwmqtt_init(lwmqtt_client_t *client, uint8_t *write_buf, size_t write_buf_size, uint8_t *read_buf,
                 size_t read_buf_size);

/**
 * Will set the network reference and callbacks for this client object.
 *
 * @param client - The client object.
 * @param ref - The reference to the network object.
 * @param read - The read callback.
 * @param write - The write callback.
 */
void lwmqtt_set_network(lwmqtt_client_t *client, void *ref, lwmqtt_network_read_t read, lwmqtt_network_write_t write);

/**
 * Will set the timer references and callbacks for this client object.
 *
 * @param client - The client object.
 * @param keep_alive_timer - The reference to the keep alive timer.
 * @param network_timer - The reference to the network timer.
 * @param set - The set callback.
 * @param get - The get callback.
 */
void lwmqtt_set_timers(lwmqtt_client_t *client, void *keep_alive_timer, void *network_timer, lwmqtt_timer_set_t set,
                       lwmqtt_timer_get_t get);

/**
 * Will set the callback used to receive incoming messages.
 *
 * @param client - The client object.
 * @param ref - A custom reference that will passed to the callback.
 * @param cb - The callback to be called.
 */
void lwmqtt_set_callback(lwmqtt_client_t *client, void *ref, lwmqtt_callback_t cb);

/**
 * The object defining the last will of a client.
 */
typedef struct {
  lwmqtt_string_t topic;
  lwmqtt_qos_t qos;
  bool retained;
  lwmqtt_string_t payload;
} lwmqtt_will_t;

/**
 * The default initializer for the will object.
 */
#define lwmqtt_default_will \
  { lwmqtt_default_string, LWMQTT_QOS0, false, lwmqtt_default_string }

/**
 * The object containing the connection options for a client.
 */
typedef struct {
  lwmqtt_string_t client_id;
  uint16_t keep_alive;
  bool clean_session;
  lwmqtt_string_t username;
  lwmqtt_string_t password;
} lwmqtt_options_t;

/**
 * The default initializer for the options object.
 */
#define lwmqtt_default_options \
  { lwmqtt_default_string, 60, true, lwmqtt_default_string, lwmqtt_default_string }

/**
 * The available return codes transported by the connack packet.
 */
typedef enum {
  LWMQTT_CONNACK_CONNECTION_ACCEPTED = 0,
  LWMQTT_CONNACK_UNACCEPTABLE_PROTOCOL = 1,
  LWMQTT_CONNACK_IDENTIFIER_REJECTED = 2,
  LWMQTT_CONNACK_SERVER_UNAVAILABLE = 3,
  LWMQTT_CONNACK_BAD_USERNAME_OR_PASSWORD = 4,
  LWMQTT_CONNACK_NOT_AUTHORIZED = 5
} lwmqtt_return_code_t;

/**
 * Will send a connect packet and wait for a connack response and set the return code.
 *
 * The network object must already be connected to the server. An error is returned if the broker rejects the
 * connection.
 *
 * @param client - The client object.
 * @param options - The options object.
 * @param will - The will object.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_connect(lwmqtt_client_t *client, lwmqtt_options_t options, lwmqtt_will_t *will,
                            lwmqtt_return_code_t *return_code, int timeout);

/**
 * Will send a publish packet and wait for all acks to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param topic - The topic.
 * @param message - The message.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_publish(lwmqtt_client_t *client, lwmqtt_string_t topic, lwmqtt_message_t msg, int timeout);

/**
 * Will send a subscribe packet with multiple topic filters plus QOS levels and wait for the suback to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param count - The number of topic filters and QOS levels.
 * @param topic_filter - The topic filters.
 * @param qos - The list of QOS levels.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_subscribe(lwmqtt_client_t *client, int count, lwmqtt_string_t *topic_filter, lwmqtt_qos_t *qos,
                              int timeout);

/**
 * Will send a subscribe packet with a single topic filter plus QOS level and wait for the suback to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param topic_filter - The topic filter.
 * @param qos - The QOS level.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_subscribe_one(lwmqtt_client_t *client, lwmqtt_string_t topic_filter, lwmqtt_qos_t qos, int timeout);

/**
 * Will send an unsubscribe packet with multiple topic filters and wait for the unsuback to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param count - The number of topic filters.
 * @param topic_filter - The topic filter.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_unsubscribe(lwmqtt_client_t *client, int count, lwmqtt_string_t *topic_filter, int timeout);

/**
 * Will send an unsubscribe packet with a single topic filter and wait for the unsuback to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param topic_filter - The topic filter.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_unsubscribe_one(lwmqtt_client_t *client, lwmqtt_string_t topic_filter, int timeout);

/**
 * Will send a disconnect packet and finish the client.
 *
 * @param client - The client object.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_disconnect(lwmqtt_client_t *client, int timeout);

/**
 * Will yield control to the client and receive incoming packets from the network.
 *
 * Applications may peek on the network if there is data available to read before calling yield and potentially block
 * until the timeout is reached. Furthermore, applications may specify the amount of bytes available to read in order
 * to constrain the yield to only receive packets that are already inflight.
 *
 * If no availability data is given the yield will return after one packet has been successfully read.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param available - The available bytes to read.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_yield(lwmqtt_client_t *client, size_t available, int timeout);

/**
 * Will yield control to the client to keep the connection alive.
 *
 * @param client - The client object.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_keep_alive(lwmqtt_client_t *client, int timeout);

#endif  // LWMQTT_H
