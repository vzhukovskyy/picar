#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wsserver/ws.h>

static void on_open(int fd) {
    char *cli;
    cli = ws_getaddress(fd);
    printf("Connection opened, client: %d | addr: %s\n", fd, cli);
    free(cli);
}

static void on_close(int fd) {
    char *cli;
    cli = ws_getaddress(fd);
    printf("Connection closed, client: %d | addr: %s\n", fd, cli);
    free(cli);
}

static void on_message(int fd, const unsigned char *msg, uint64_t size, int type) {
    char *cli;
    cli = ws_getaddress(fd);
    printf("I receive a message: %s (%llu), from: %s/%d\n", msg, size, cli, fd);

    sleep(2);
    ws_sendframe_txt(fd, "hello", false);
    sleep(2);
    ws_sendframe_txt(fd, "world", false);

    free(cli);
}

void* ws_server_serve(void *arg) {
    struct ws_events evs;
    evs.onopen    = &on_open;
    evs.onclose   = &on_close;
    evs.onmessage = &on_message;

    ws_socket(&evs, 8089);

    return NULL;
}
