#include <stdio.h>
#include <string.h>
#include "servkit.h"

static SERVKIT_SERVER server;
static SERVKIT_CLIENT client;

void client_accept(SERVKIT_CLIENT __client)
{
    printf("Client connected to server\n");
    client = __client;
}

void client_handler(SERVKIT_CLIENT __client, uint8_t *data, int len, void *arg)
{

}

void event(void *arg)
{
    char msg[] = "Hello, world!";
    int len = strlen(msg) + 1;
    if (client) {
        printf("Writing to client\n");
        servkit_server_send_raw(server, client, msg, len);
    }
}

int main(void)
{
    servkit_init();

    server = servkit_server_open("/tmp/server.sock", client_handler, NULL);
    if (server) {
        printf("Opened servkit server successfully\n");
    }

    servkit_server_install_connect_cb(server, client_accept);

    servkit_addtimer(1000, SERVKIT_TIMER_RETRIGGER, event, NULL);

    servkit_loop();

    return 0;
}
