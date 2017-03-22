#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/un.h>
#include <fcntl.h>
#include <string.h>

#include "servkit.h"

#define PATH_SOCKET     "/tmp/server.sock"
#define MAXLEN          2048

/* Global variables */
static int iface;

/* Parse message received from server */
void parse_cmd(int fd, void *cmd)
{

}

/* Read-callback for interface socket */
void server_read(int fd, short revents, void *arg)
{
    char cmd[MAXLEN];
    int ret = 0;

    ret = read(fd, cmd, (size_t)MAXLEN);
    if (ret > 0) {
        cmd[ret] = (char)0;
        parse_cmd(fd, cmd);
    }
}

/* Error-callback for interface socket */
void server_error(int fd, short revents, void *arg)
{
    fprintf(stderr, "Error occured with server: %s\n", strerror(errno));
    exit(5);
}

int main(int argc, const char *argv[])
{
    struct sockaddr_un socket_client;
    char hello[64] = "Hello, there!\n";
    int s_client;

    /* Initialize libevquick */
    evquick_init();

    /* Create client socket */
    s_client = socket(AF_UNIX, SOCK_STREAM, 0);
    if (s_client < 0) {
        fprintf(stderr, "Could not create client socket\n");
        return -1;
    }

    /* Connect to server socket */
    socket_client.sun_family = AF_UNIX;
    strncpy(socket_client.sun_path, PATH_SOCKET, UNIX_PATH_MAX);
    if (connect(s_client, (struct sockaddr *)&socket_client, sizeof(struct sockaddr_un))) {
        fprintf(stderr, "Failed to connect to server socket at: %s\n", PATH_SOCKET);
        return -1;
    }

    /* Store interface with server in global variable */
    iface = s_client;

    /* Install callback for events triggered by server */
    evquick_addevent(iface, EVQUICK_EV_READ, server_read, server_error, NULL);

    /* Send a hello message */
    write(iface, hello, MAXLEN);

    /* Enter libevquick event-loop */
    evquick_loop();
}
