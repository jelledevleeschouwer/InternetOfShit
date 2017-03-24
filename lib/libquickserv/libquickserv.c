#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/un.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "libevquick.h"

/* Libquickserv */
#include "libquickserv.h"

#ifdef DBG_QUICKSERV
    #define dbg printf
#else
    #define dbg(...)
#endif

#define QUICKSERV_FLAGMASK (0x4000)

/* Quickserv client */
struct quickserv_client_s
{
    int iface;
    QUICKSERV_SERVER server;
};

/* Quickserv server */
struct quickserv_server_s
{
    uint16_t num_clients;
    quickserv_client_handler_cb client_handler;
    quickserv_client_accept_cb accept_handler;
    quickserv_server_error_cb error_handler;
    QUICKSERV_CLIENT client; /* TODO: For now only 1 client is supported, update to list of clients */
    void *arg;
};

/* For transmitting packets */
static uint8_t pktbuf[QUICKSERV_SIZE_MAX];

/**************************************************************************************************
 * Servkit
 **************************************************************************************************/

void quickserv_init(void)
{
    evquick_init();
}

void quickserv_loop(void)
{
    evquick_loop();
}

void quickserv_addtimer(time_t interval, short flags, void (*callback)(void *arg), void *arg)
{
    evquick_addtimer(interval, flags & QUICKSERV_FLAGMASK, callback, arg);
}

/**************************************************************************************************
 * Server related
 **************************************************************************************************/

static void server_error(void *arg)
{
    QUICKSERV_SERVER server = (QUICKSERV_SERVER)arg;
    if (server->error_handler) {
        server->error_handler(server);
    }
}

/* Read-callback for client socket */
static void client_read(int fd, short revents, void *arg)
{
    QUICKSERV_CLIENT src = (QUICKSERV_CLIENT)arg;
    char data[QUICKSERV_SIZE_MAX];
    int ret = 0;

    ret = read(fd, data, (size_t)QUICKSERV_SIZE_MAX);
    if (ret > 0) {
        src->server->client_handler(src, (uint8_t *)data, ret, src->server->arg);
    }
}

/* Error-callback for client socket */
static void client_error(int fd, short revents, void *arg)
{
    QUICKSERV_CLIENT client = (QUICKSERV_CLIENT)arg;
    QUICKSERV_SERVER self = client->server;
    fprintf(stderr, "Client closed connection\n");
    close(fd);
    self->num_clients--;
    server_error((void *)self);
    free(self->client);
    self->client = NULL;
}

/* Setup server-client interface */
static void serve_client(QUICKSERV_SERVER server, int fd)
{
    QUICKSERV_CLIENT new = NULL;

    dbg("Serving new client...\n");

    // Only accept one client for now
    if (server->num_clients == 1 || server->client) {
        close(fd);
        return;
    }

    new = (QUICKSERV_CLIENT)malloc(sizeof(struct quickserv_client_s));
    if (!new) {
        return;
    } else {
        new->iface = fd;
        new->server = server;
    }

    // Assign client to server
    server->num_clients++;
    server->client = new;

    /* Notify application of succesfull connection with client */
    if (server->accept_handler) {
        server->accept_handler(new);
    }

    evquick_addevent(fd, EVQUICK_EV_READ, client_read, client_error, (void *)new);
}

/* Read-callback for passive socket */
static void connect_accept(int fd, short revents, void *arg)
{
    socklen_t socklen = sizeof(struct sockaddr_un);
    struct sockaddr_un client;
    int conn_fd;

    conn_fd = accept(fd, (struct sockaddr *)&client, &socklen);
    if (conn_fd >= 0) {
        serve_client((QUICKSERV_SERVER)arg, conn_fd);
    }
}

/* Error-callback for passive socket */
static void connect_error(int fd, short revents, void *arg)
{
    fprintf(stderr, "Failed accepting a new connection: %s\n", strerror(errno));
    server_error(arg);
}

// Open a server instance at a certain path
QUICKSERV_SERVER quickserv_server_open(const char *path, quickserv_client_handler_cb handler, void *arg)
{
    QUICKSERV_SERVER server = (QUICKSERV_SERVER)malloc(sizeof(struct quickserv_server_s));
    struct sockaddr_un socket_server;
    int s_server;

    /* Create Server socket */
    s_server = socket(AF_UNIX, SOCK_STREAM, 0);

    /* Bind socket to path */
    socket_server.sun_family = AF_UNIX;
    strcpy(socket_server.sun_path, path);
    unlink(socket_server.sun_path);
    bind(s_server, (struct sockaddr *)&socket_server, sizeof(socket_server));

    /* Configure server instance */
    if (!server) {
        return NULL;
    } else {
        server->client_handler = handler;
        server->accept_handler = NULL;
        server->error_handler = NULL;
        server->num_clients = 0;
        server->client = NULL;
        server->arg = arg;
    }

    /* Start listening on socket and add read-event */
    listen(s_server, 3);
    evquick_addevent(s_server, EVQUICK_EV_READ, connect_accept, connect_error, (void *)server);

    return server;
}

// Send from a server to a particular client
int quickserv_server_send(QUICKSERV_SERVER server, QUICKSERV_CLIENT client, uint8_t *data, int len)
{
    memcpy(pktbuf, data, len);
    return write(client->iface, pktbuf, len);
}

// Install a callback-function for detecting new connections with clients
int quickserv_server_install_connect_cb(QUICKSERV_SERVER server, quickserv_client_accept_cb handler)
{
    if (server) {
        server->accept_handler = handler;
        return 0;
    }
    return -1;
}

// Install a callback-function for catching errors
int quickserv_server_install_error_cb(QUICKSERV_SERVER server, quickserv_server_error_cb handler)
{
    if (server) {
        server->error_handler = handler;
        return 0;
    }
    return -1;
}

/**************************************************************************************************
 * Client related
 **************************************************************************************************/

// Connect a client to a server at a particular path
QUICKSERV_CLIENT quickserv_client_connect(const char *path, quickserv_server_handler_cb handler)
{
    return NULL;
}

// Send from a client to it's connected server
int quickserv_client_send(QUICKSERV_CLIENT client, uint8_t *data, int len)
{
    return 0;
}

