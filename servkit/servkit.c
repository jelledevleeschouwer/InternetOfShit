#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/un.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "libevquick.h"

/* Servkit */
#include "servkit.h"

#ifdef DBG_SERVKIT
    #define dbg printf
#else
    #define dbg(...)
#endif

#define SERVKIT_FLAGMASK        (0x4000)     // 0b0100000000000000

// Message types
#define SERVKIT_TYPE_DATA       (0)
#define SERVKIT_TYPE_LOGIN      (1)
#define SERVKIT_TYPE_COMMAND    (2)

// Commands
#define SERVKIT_CMD_SHUTDOWN    (0xFFFF)

PACKED_STRUCT_DEF servkit_msg {
    uint16_t type;
    union servkit_contents {
        struct servkit_data {
            uint16_t len;
            uint8_t buf[0];
        } data;
        struct servkit_login {
            uint16_t len;
            uint8_t buf[0];
        } login;
        struct servkit_command {
            uint16_t cmd;
        } command;
    } contents;
};

struct servkit_client_s {
    int iface;
    SERVKIT_SERVER server;
};

struct servkit_server_s {
    uint16_t num_clients;
    servkit_client_handler_cb client_handler;
    servkit_client_accept_cb accept_handler;
    servkit_server_error_cb error_handler;
    SERVKIT_CLIENT client; /* TODO: For now only 1 client is supported, update to list of clients */
    void *arg;
};

/* For transmitting packets */
static uint8_t pktbuf[SERVKIT_SIZE_MAX];

/**************************************************************************************************
 * Servkit
 **************************************************************************************************/

void servkit_init(void)
{
    evquick_init();
}

void servkit_loop(void)
{
    evquick_loop();
}

void servkit_addtimer(time_t interval, short flags, void (*callback)(void *arg), void *arg)
{
    evquick_addtimer(interval, flags & SERVKIT_FLAGMASK, callback, arg);
}

/**************************************************************************************************
 * Server related
 **************************************************************************************************/

static void server_error(void *arg)
{
    SERVKIT_SERVER server = (SERVKIT_SERVER)arg;
    if (server->error_handler) {
        server->error_handler(server);
    }
}

/* Parse message received from client */
static void server_parse_cmd(void *cmd, SERVKIT_CLIENT src)
{
    struct servkit_msg *msg = cmd;
    switch (ntohs(msg->type)) {
        case SERVKIT_TYPE_DATA:
            /* Deliver to application */
            src->server->client_handler(src, msg->contents.data.buf, ntohs(msg->contents.data.len), src->server->arg);
            break;
        case SERVKIT_TYPE_LOGIN:
            break;
        case SERVKIT_TYPE_COMMAND:
        default:
            fprintf(stderr, "Unknown message type\n");
    }
}

/* Read-callback for client socket */
static void client_read(int fd, short revents, void *arg)
{
    char cmd[SERVKIT_SIZE_MAX];
    int ret = 0;

    ret = read(fd, cmd, (size_t)SERVKIT_SIZE_MAX);
    if (ret > 0) {
        cmd[ret] = (char)0;
        server_parse_cmd(cmd, (SERVKIT_CLIENT)arg);
    }
}

/* Error-callback for client socket */
static void client_error(int fd, short revents, void *arg)
{
    SERVKIT_CLIENT client = (SERVKIT_CLIENT)arg;
    SERVKIT_SERVER self = client->server;
    fprintf(stderr, "Client closed connection\n");
    close(fd);
    self->num_clients--;
    server_error((void *)self);
    free(self->client);
    self->client = NULL;
}

/* Setup server-client interface */
static void serve_client(SERVKIT_SERVER server, int fd)
{
    SERVKIT_CLIENT new = NULL;

    dbg("Serving new client...\n");

    // Only accept one client for now
    if (server->num_clients == 1 || server->client) {
        close(fd);
        return;
    }

    new = (SERVKIT_CLIENT)malloc(sizeof(struct servkit_client_s));
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
        serve_client((SERVKIT_SERVER)arg, conn_fd);
    }
}

/* Error-callback for passive socket */
static void connect_error(int fd, short revents, void *arg)
{
    fprintf(stderr, "Failed accepting a new connection: %s\n", strerror(errno));
    server_error(arg);
}

// Open a server instance at a certain path
SERVKIT_SERVER servkit_server_open(const char *path, servkit_client_handler_cb handler, void *arg)
{
    SERVKIT_SERVER server = (SERVKIT_SERVER)malloc(sizeof(struct servkit_server_s));
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
int servkit_server_send(SERVKIT_SERVER server, SERVKIT_CLIENT client, uint8_t *data, int len)
{
    struct servkit_msg *msg = (struct servkit_msg *)pktbuf;
    msg->type = htons(SERVKIT_TYPE_DATA);
    msg->contents.data.len = htons(len);
    memcpy(msg->contents.data.buf, data, len);
    return write(client->iface, pktbuf, SERVKIT_SIZE_MAX);
}

// If client is not a Servkit-client there is a chance (99%) that it doesn't support the Servkit
// message format, therefor this function can send RAW data without the Servkit message format.
int servkit_server_send_raw(SERVKIT_SERVER server, SERVKIT_CLIENT client, uint8_t *data, int len)
{
    memcpy(pktbuf, data, len);
    return write(client->iface, pktbuf, len);
}

// Install a callback-function for detecting new connections with clients
int servkit_server_install_connect_cb(SERVKIT_SERVER server, servkit_client_accept_cb handler)
{
    if (server) {
        server->accept_handler = handler;
        return 0;
    }
    return -1;
}

// Install a callback-function for catching errors
int servkit_server_install_error_cb(SERVKIT_SERVER server, servkit_server_error_cb handler)
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
SERVKIT_CLIENT servkit_client_connect(const char *path, servkit_server_handler_cb handler)
{
    return NULL;
}

// Send from a client to it's connected server
int servkit_client_send(SERVKIT_CLIENT client, uint8_t *data, int len)
{
    return 0;
}

