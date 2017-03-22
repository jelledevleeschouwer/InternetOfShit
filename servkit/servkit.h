#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#ifndef __SERVKIT_H
#define __SERVKIT_H

#define PACKED_STRUCT_DEF struct __attribute__((packed))

/* ServKit */
#define SERVKIT_SIZE_MAX     (2048)

struct servkit_server_s;
typedef struct servkit_server_s* SERVKIT_SERVER;

struct servkit_client_s;
typedef struct servkit_client_s* SERVKIT_CLIENT;

/**************************************************************************************************
 * Servkit
 **************************************************************************************************/
void servkit_init(void);
void servkit_loop(void);

#define SERVKIT_TIMER_RETRIGGER 0x4000

// Wrapper for libevquick-timers
void servkit_addtimer(time_t interval, short flags, void (*callback)(void *arg), void *arg);

/**************************************************************************************************
 * Server related
 **************************************************************************************************/
typedef void (* servkit_client_handler_cb)(SERVKIT_CLIENT client, uint8_t *data, int len, void *arg);
typedef void (* servkit_client_accept_cb)(SERVKIT_CLIENT client);
typedef void (* servkit_server_error_cb)(SERVKIT_SERVER server);

// Open a server instance at a certain path
SERVKIT_SERVER servkit_server_open(const char *path, servkit_client_handler_cb handler, void *arg);

// Send from a server to a particular client
int servkit_server_send(SERVKIT_SERVER server, SERVKIT_CLIENT client, uint8_t *data, int len);
// If client is not a Servkit-client there is a chance (99%) that it doesn't support the Servkit
// message format, therefor this function can send RAW data without the Servkit message format.
int servkit_server_send_raw(SERVKIT_SERVER server, SERVKIT_CLIENT client, uint8_t *data, int len);
int servkit_server_install_connect_cb(SERVKIT_SERVER server, servkit_client_accept_cb handler);
int servkit_server_install_error_cb(SERVKIT_SERVER server, servkit_server_error_cb handler);

/**************************************************************************************************
 * Client related
 **************************************************************************************************/
typedef void (* servkit_server_handler_cb)(SERVKIT_CLIENT client, uint8_t *data, int len, void *arg);

// Connect a client to a server at a particular path
SERVKIT_CLIENT servkit_client_connect(const char *path, servkit_server_handler_cb handler);

// Send from a client to it's connected server
int servkit_client_send(SERVKIT_CLIENT client, uint8_t *data, int len);

#endif /* __SERVKIT_H */

