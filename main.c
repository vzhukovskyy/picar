#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>

void* http_server_serve(void *arg);
void http_server_stop();
void* ws_server_serve(void *arg);

static sem_t exit_semaphore;

static void signal_handler(int sig) {
    printf("main: got signal %d (%s)", sig, strsignal(sig));
    sem_post(&exit_semaphore);
}

int main(int argc, char** argv) {
    sem_init(&exit_semaphore, 0, 0);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);

    pthread_t http_server_thread, ws_server_thread;
    pthread_create(&http_server_thread, NULL, http_server_serve, NULL);
    pthread_create(&ws_server_thread, NULL, ws_server_serve, NULL);

    sem_wait(&exit_semaphore);

    http_server_stop();
    // how to shutdown websocket server?

    pthread_join(http_server_thread, NULL);
    // pthread_join(ws_server_thread, NULL);

    return 0;
}
