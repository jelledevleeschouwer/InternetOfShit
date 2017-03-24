#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <inttypes.h>

#ifndef __QUICKSERV_H_
#define __QUICKSERV_H_

/* Macro's */
#define PACKED_STRUCT_DEF struct __attribute__((packed))

/* Libquickserv */
#define QUICKSERV_SIZE_MAX     (2048)

struct quickserv_server_s;
typedef struct quickserv_server_s* QUICKSERV_SERVER;

struct quickserv_client_s;
typedef struct quickserv_client_s* QUICKSERV_CLIENT;

/**************************************************************************************************
 * Servkit
 **************************************************************************************************/
void quickserv_init(void);
void quickserv_loop(void);

#define QUICKSERV_TIMER_RETRIGGER 0x4000

// Wrapper for libevquick-timers
void quickserv_addtimer(time_t interval, short flags, void (*callback)(void *arg), void *arg);

/**************************************************************************************************
 * Server related
 **************************************************************************************************/
typedef void (* quickserv_client_handler_cb)(QUICKSERV_CLIENT client, uint8_t *data, int len, void *arg);
typedef void (* quickserv_client_accept_cb)(QUICKSERV_CLIENT client);
typedef void (* quickserv_server_error_cb)(QUICKSERV_SERVER server);

// Open a server instance at a certain path
QUICKSERV_SERVER quickserv_server_open(const char *path, quickserv_client_handler_cb handler, void *arg);

// Install a callback-function for detecting new connections with clients
int quickserv_server_install_connect_cb(QUICKSERV_SERVER server, quickserv_client_accept_cb handler);

// Install a callback-function for catching errors
int quickserv_server_install_error_cb(QUICKSERV_SERVER server, quickserv_server_error_cb handler);

// Send from a server to a particular client
int quickserv_server_send(QUICKSERV_SERVER server, QUICKSERV_CLIENT client, uint8_t *data, int len);

/**************************************************************************************************
 * Client related
 **************************************************************************************************/
typedef void (* quickserv_server_handler_cb)(QUICKSERV_CLIENT client, uint8_t *data, int len, void *arg);

// Connect a client to a server at a particular path
QUICKSERV_CLIENT quickserv_client_connect(const char *path, quickserv_server_handler_cb handler);

// Send from a client to it's connected server
int quickserv_client_send(QUICKSERV_CLIENT client, uint8_t *data, int len);

#endif /* __QUICKSERV_H_ */

