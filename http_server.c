#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/stat.h>

static int PORT = 8088;
#define HTML_FILENAME "index.html"
static volatile int stopped = 0;

static char buffer[2048];

int receive_request(int fd, const char** method, const char** url);
#define socket_print(socket_fd, string) send(socket_fd, string, strlen(string), 0)

void* http_server_serve(void *arg) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("ERROR opening socket: %s", strerror(errno));
        return NULL;
    }
    
    int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)PORT);
    if (bind(server_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        printf("ERROR on binding: %s", strerror(errno));
        return NULL;
    }
    
    printf("http_server: listening on port %d", PORT);

    if (listen(server_fd, 1) < 0) {
        printf("ERROR on listen: %s", strerror(errno));
        return NULL;
    }
    
    while (stopped == 0) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(server_fd, &fds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int res = select(server_fd+1, &fds, NULL, NULL, &tv);
        if (res <= 0)
            continue;

        printf("http_server: client connected");

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            printf("ERROR in accept: %s", strerror(errno));
            continue;
        }

        printf("http_server: client accepted");
        
        const char *method, *url;
        int received = receive_request(client_fd, &method, &url);
        if (received <= 0) {
            printf("http_server: no request");
            close(client_fd);
            continue;
        }
        printf("http_server: request %s %s", method, url);

        if (strcmp(method, "GET") || strcmp(url, "/")) {
            socket_print(client_fd, 
                "HTTP/1.1 404 Not Found\r\n"
                "\r\n");
            close(client_fd);
            continue;
        }

        struct stat st;
        stat(HTML_FILENAME, &st);
        int size = st.st_size;

        sprintf(buffer, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "\r\n", size);
        socket_print(client_fd, buffer);

        FILE* f = fopen("index.html", "r");
        do {
            int read = fread(buffer, 1, sizeof(buffer), f);
            socket_print(client_fd, buffer);
            size -= read;
        } while (size > 0);
        fclose(f);

        close(client_fd);
    }

    close(server_fd);
    printf("http_server: server shut down");
    return NULL;
}

int receive_request(int fd, const char** method, const char **url) {
    *method = NULL;
    *url = NULL;

    char *buf = buffer;
    int buf_len = sizeof(buffer);

    printf("http_server: receiving request");
    while (!stopped) {
        int recvd = recv(fd, buf, buf_len-1, MSG_DONTWAIT);
        if (recvd <= 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(100);
                continue;
            } else {
                printf("http_thread: error %d(%s) receiving request", errno, strerror(errno));
                return 0;
            }
        }

        buf[recvd] = 0;

        char *p1 = strchr(buf, ' ');
        if (p1 != NULL) {
            *p1 = 0;
            *method = buf;
            char *p2 = strchr(p1+1, '\r');
            if (p2 != NULL) {
                *(p2-9) = 0;  // before HTTP1.1
                *url = p1+1;
                return 1;
            }
        }
        printf("http_server: cannot parse request");
        return 0;
    }

    printf("http_server: request not received");
    return 0;
}

void http_server_stop() {
    stopped = 1;
}
