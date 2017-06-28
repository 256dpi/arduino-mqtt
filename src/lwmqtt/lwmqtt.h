#ifndef LWMQTT_H
#define LWMQTT_H

#include <stdbool.h>

/**
 * The error type used by all exposed APIs.
 */
typedef enum {
  LWMQTT_SUCCESS = 0,
  LWMQTT_UNANSWERED_PIN = -1,
  LWMQTT_BUFFER_TOO_SHORT = -2,
  LWMQTT_REMAINING_LENGTH_OVERFLOW = -3,
  LWMQTT_LENGTH_MISMATCH = -4,
  LWMQTT_NOT_ENOUGH_DATA = -5,
  LWMQTT_NETWORK_CONNECT_ERROR = -6,
  LWMQTT_NETWORK_READ_ERROR = -7,
  LWMQTT_NETWORK_WRITE_ERR = -8,
  LWMQTT_NO_OR_WRONG_PACKET = -9,
  LWMQTT_CONNECTION_DENIED = -10,
  LWMQTT_DECODE_ERROR = -11
} lwmqtt_err_t;

/**
 * A multi value string. Can be either a C string or a length prefixed string.
 */
typedef struct {
  int len;
  char *data;
} lwmqtt_string_t;

/**
 * The initializer for string objects.
 */
#define lwmqtt_default_string \
  { 0, NULL }

/**
 * Return a string object for the passed C string.
 *
 * @param str - The C string.
 * @return A string object.
 */
lwmqtt_string_t lwmqtt_str(const char *str);

/**
 * Compares a string object to a C string.
 *
 * @param a - The string object to compare.
 * @param b - The C string to compare.
 * @return Similarity e.g. strcmp().
 */
int lwmqtt_strcmp(lwmqtt_string_t *a, char *b);

/**
 * The available QOS levels.
 */
typedef enum { LWMQTT_QOS0 = 0, LWMQTT_QOS1 = 1, LWMQTT_QOS2 = 2 } lwmqtt_qos_t;

/**
 * The message object used to publish and receive messages.
 */
typedef struct {
  lwmqtt_qos_t qos;
  bool retained;
  void *payload;
  int payload_len;
} lwmqtt_message_t;

/**
 * The initializer for messages objects.
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
 * timeout and wait for more incoming data. It may set read to zero if no data is has been read.
 */
typedef lwmqtt_err_t (*lwmqtt_network_read_t)(lwmqtt_client_t *c, void *ref, unsigned char *buf, int len, int *read,
                                              unsigned int timeout);

/**
 * The callback used to write to a network object.
 *
 * The callback is expected to write up to the amount of bytes from the passed buffer. It should wait up to the
 * specified timeout to write the specified data to the network.
 */
typedef lwmqtt_err_t (*lwmqtt_network_write_t)(lwmqtt_client_t *c, void *ref, unsigned char *buf, int len, int *sent,
                                               unsigned int timeout);

/**
 * The callback used to set a timer.
 */
typedef void (*lwmqtt_timer_set_t)(lwmqtt_client_t *c, void *ref, unsigned int timeout);

/**
 * The callback used to get a timers value.
 */
typedef unsigned int (*lwmqtt_timer_get_t)(lwmqtt_client_t *c, void *ref);

/**
 * The callback used to forward incoming messages.
 *
 * Note: The callback is mostly executed because of a call to lwmqtt_yield() that processes incoming messages. However,
 * it is possible that the callback is also executed during a call to lwmqtt_subscribe(), lwmqtt_publish() or
 * lwmqtt_unsubscribe() if incoming messages are received between the required acknowledgements. It is therefore not
 * recommended to call any further lwmqtt methods in the callback as this might result in weird call stacks. The
 * callback should place the received messages in a queue and dispatch them after the caller has returned.
 */
typedef void (*lwmqtt_callback_t)(lwmqtt_client_t *, void *ref, lwmqtt_string_t *, lwmqtt_message_t *);

/**
 * The client object.
 */
struct lwmqtt_client_t {
  unsigned short next_packet_id;
  unsigned int keep_alive_interval;
  bool ping_outstanding;

  int write_buf_size, read_buf_size;
  unsigned char *write_buf, *read_buf;

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
void lwmqtt_init(lwmqtt_client_t *client, unsigned char *write_buf, int write_buf_size, unsigned char *read_buf,
                 int read_buf_size);

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
 * Will set the timer references and callbacks for this client objects.
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
  lwmqtt_message_t message;
} lwmqtt_will_t;

/**
 * The default initializer for the will object.
 */
#define lwmqtt_default_will \
  { lwmqtt_default_string, lwmqtt_default_message }

/**
 * The object containing the connections options for a client.
 */
typedef struct {
  lwmqtt_string_t client_id;
  unsigned short keep_alive;
  bool clean_session;
  lwmqtt_string_t username;
  lwmqtt_string_t password;
} lwmqtt_options_t;

/**
 * The default initializer for the options object.
 */
#define lwmqtt_default_options \
  { lwmqtt_default_string, 60, 1, lwmqtt_default_string, lwmqtt_default_string }

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
lwmqtt_err_t lwmqtt_connect(lwmqtt_client_t *client, lwmqtt_options_t *options, lwmqtt_will_t *will,
                            lwmqtt_return_code_t *return_code, unsigned int timeout);

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
lwmqtt_err_t lwmqtt_publish(lwmqtt_client_t *client, const char *topic, lwmqtt_message_t *msg, unsigned int timeout);

/**
 * Will send a subscribe packet with a single topic filter plus qos level and wait for the suback to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param topic_filter - The topic filter.
 * @param qos - The QoS level.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_subscribe(lwmqtt_client_t *client, const char *topic_filter, lwmqtt_qos_t qos,
                              unsigned int timeout);

/**
 * Will send an unsubscribe packet and wait for the unsuback to complete.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param topic_filter - The topic filter.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_unsubscribe(lwmqtt_client_t *client, const char *topic_filter, unsigned int timeout);

/**
 * Will send a disconnect packet and finish the client.
 *
 * @param client - The client object.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_disconnect(lwmqtt_client_t *client, unsigned int timeout);

/**
 * Will yield control to the client and receive incoming packets from the network.
 *
 * Applications may peek on the network if there is data available to read before calling yield and potentially block
 * until the timeout is reached. Furthermore, applications may specify the amount of bytes available to read in order
 * to constrain the yield to only receive packets that are already inflight.
 *
 * Note: The message callback might be called with incoming messages as part of this call.
 *
 * @param client - The client object.
 * @param available - The available bytes to read.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_yield(lwmqtt_client_t *client, unsigned int available, unsigned int timeout);

/**
 * Will yield control to the client to keep the connection alive.
 *
 * @param client - The client object.
 * @param timeout - The command timeout.
 * @return An error value.
 */
lwmqtt_err_t lwmqtt_keep_alive(lwmqtt_client_t *client, unsigned int timeout);

#endif  // LWMQTT_H
