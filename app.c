/**************************************************************************************************
 * app.c
 *
 * Description
 * ===========
 *  Server-instance that serves ADC (MCP3008) data to clients. Uses Servkit IPC-communication.
 *
 * Details
 * =======
 *  Author: Jelle De Vleeschouwer
 *  Copyright (C) 2017 Jelle De Vleeschouwer
 *
 **************************************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "mcp3008.h"
#include "libquickserv.h"

/* Utilities */
#define EVER                    (;;)
#define IGNORE(x)               (void)(x)
#define ZALLOC(size)            calloc(1, (size_t)(size));
#define ERROR(var)              fprintf(stderr, var)

#ifdef DBG_APP
    #define dbg printf
#else
    #define dbg(...)
#endif

/* Defaults */
#define POLL_TIMEOUT            (5)
#define SEAT_PRESSURE_TRESH     (512) // [0, 1023]
#define PAPER_PRESSURE_TRESH    (512) // [0, 1023]
#define SERVER_PATH             "/tmp/server.sock"

/* Constants */
#define PAPER_CHANNEL           (MCP3008_CH0)
#define SEAT_CHANNEL            (MCP3008_CH4)
#define MSG_LEN                 (64)

/* Static variables */
static int paper_pressure_tresh = 0;
static int seat_pressure_tresh = 0;
static time_t poll_timeout = 0;
static char *server_path = NULL;

static QUICKSERV_SERVER server = NULL;
static QUICKSERV_CLIENT client = NULL;

/* Callback when new client is accepted */
static void client_accept(QUICKSERV_CLIENT __client)
{
    measure(NULL);
    printf("Client connected to server\n");
    client = __client;
}

/* Callback when new data is received from clients */
static void client_handler(QUICKSERV_CLIENT __client, uint8_t *data, int len, void *arg)
{
    /* XXX: NOT IMPLEMENTED: we don't accept requests from client ATM. */
}

/* Send an actual message throug quickserv if client is connected */
static void server_send_notification(char *msg)
{
    int ret = 0;
    if (client) {
        ret = quickserv_server_send(server, client, (uint8_t *)msg, strlen(msg));
        if (ret == strlen(msg)) {
            dbg("INFO: Sent notification to client: SUCCESS\n");
        } else {
            ERROR("ERROR: Failed to send notification to client\n");
            exit(255);
        }
    } else {
        dbg("INFO: No client connected, cannot send notification\n");
    }
}

/* Notify a change in seat pressure to main application */
static void notify_seat_change(int seat_occupied)
{
    static int seat_state = 0;
    char msg[MSG_LEN] = "{ \"SEAT\" : ";
    if (seat_state != seat_occupied) {
        snprintf(msg + strlen(msg), MSG_LEN, "\"%d\" }", seat_occupied);
        server_send_notification(msg);
        dbg("Notify: %s\n", msg);
    }
    seat_state = seat_occupied;
}

/* Notify a change in paper pressure to main application */
static void notify_paper_change(int paper_available)
{
    static int paper_state = 0;
    char msg[MSG_LEN] = "{ \"PAPER\" : ";
    if (paper_state != paper_available) {
        snprintf(msg + strlen(msg), MSG_LEN, "\"%d\" }", paper_available);
        server_send_notification(msg);
        dbg("Notify: %s\n", msg);
    }
    paper_state = paper_available;
}

/* Measure pressure of toilet paper on the right ADC channel */
static uint16_t toilet_paper_pressure(void)
{
    return mcp3008_readChannel(PAPER_CHANNEL, MCP3008_SE);
}

/* Measure pressure of toilet seat on the right ADC channel */
static uint16_t toilet_seat_pressure(void)
{
    return mcp3008_readChannel(SEAT_CHANNEL, MCP3008_SE);
}

/* Do the mathzzz.... *cugh* */
static void parse_pressures(uint16_t seat_pressure, uint16_t paper_pressure)
{
    if (seat_pressure < seat_pressure_tresh) {
        notify_seat_change(0); // 1: occupied
    } else {
        notify_seat_change(1); // 0: not occupied
    }

    if (paper_pressure < paper_pressure_tresh) {
        notify_paper_change(0); // 0: paper not available
    } else {
        notify_paper_change(1); // 1: paper available
    }
}

/* Print out a help menu */
static void help(void)
{
    printf("Usage:\n    ./app [options]\n\n");
    printf("Options:\n\n");
    printf("   -h               : Print out this help menu\n");
    printf("   -f   filepath    : Path where server has to open it's socket\n");
    printf("   -t   timeout     : Interval of 'timout' seconds between measurements\n");
    printf("   -s   seat_tresh  : Treshold of seat pressure before sitting is detected\n");
    printf("   -p   paper_tresh : Treshold of paper pressure before paper is detected\n");
    printf("\n");
}

/* Changes server path dynamically (!!!TOTALLY UNSAFE!!!) #Yolo */
static void set_server_path(const char *path)
{
    if (server_path)
        free(server_path);
    server_path = ZALLOC(strlen(path) + 1);
    strcpy(server_path, path);
}

/* Parse command-line options */
static void parse_options(int argc, char * const argv[])
{
    int c = 0;

    poll_timeout = POLL_TIMEOUT;
    seat_pressure_tresh = SEAT_PRESSURE_TRESH;
    paper_pressure_tresh = PAPER_PRESSURE_TRESH;
    set_server_path(SERVER_PATH);

    opterr = 0;
    while ((c = getopt(argc, argv, "t:s:p:h")) != -1) {
        switch(c) {
            case 'f':
                set_server_path(optarg);
                dbg("Setting server path: %s\n", server_path);
                break;
            case 't':
                poll_timeout = atoi(optarg);
                dbg("Setting timeout: %d\n", (int)poll_timeout);
                break;
            case 's':
                seat_pressure_tresh = atoi(optarg);
                dbg("Setting seat pressure treshold: %d\n", seat_pressure_tresh);
                break;
            case 'p':
                paper_pressure_tresh = atoi(optarg);
                dbg("Setting paper pressure treshold: %d\n", paper_pressure_tresh);
                break;
            case 'h':
                help();
                exit(0);
                break;
            case '?':
                if (optopt == 't' || optopt == 's' || optopt == 'p')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else
                    fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
                break;
            default:
                help();
                exit(0);
        }
    }
}

/* Do a new measurement and parse results */
void measure(void *arg)
{
    parse_pressures(toilet_paper_pressure(), toilet_seat_pressure());
}

int main(int argc, char * const argv[])
{
    /* Parse command-line arguments */
    parse_options(argc, argv);

    /* Initialize interface for SPI with MCP3008 */
    if (mcp3008_init(16)) {
        fprintf(stderr, "Could not initialize SPI interface with A/D converter\n");
        exit(255);
    }

    /* Initialize quickserv */
    quickserv_init();

    /* Open quickserv-server */
    server = quickserv_server_open(server_path, client_handler, NULL);
    if (server) {
        dbg("Open server succesfully at: %s\n", server_path);
    }

    /* Install callback to capture new connections */
    quickserv_server_install_connect_cb(server, client_accept);

    /* Trigger measurement every 'timeout' seconds */
    quickserv_addtimer(1000 * poll_timeout, QUICKSERV_TIMER_RETRIGGER, measure, NULL);

    /* Enter libevquick event loop */
    quickserv_loop();

    /* Cleanup resources required for MCP3008 */
    mcp3008_stop();

    return 0;
}

/**************************************************************************************************
 *  app.c
 **************************************************************************************************/
