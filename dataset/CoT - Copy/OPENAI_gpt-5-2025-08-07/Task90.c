#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>

// Step 1-5: Secure multi-client chat server in C with basic broadcast, limits, timeouts, sanitize.

#define MAX_CLIENTS 16
#define MAX_MSG_LEN 1024
#define BACKLOG 50
#define SOCKET_TIMEOUT_SEC 15

typedef struct {
    int srv_fd;
    int running;
    int clients[MAX_CLIENTS];
    int client_count;
    pthread_mutex_t lock;
    pthread_t accept_thread;
} c_server_t;

static void safe_close(int fd) {
    if (fd >= 0) close(fd);
}

static void set_rcv_timeout(int fd, int sec) {
    struct timeval tv;
    tv.tv_sec = sec; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static void broadcast_msg(c_server_t *srv, const char *msg, int src_fd) {
    if (!msg || !*msg) return;
    char sanitized[MAX_MSG_LEN + 1];
    size_t j = 0;
    for (size_t i = 0; msg[i] && j < MAX_MSG_LEN; ++i) {
        unsigned char c = (unsigned char)msg[i];
        if (c >= 32 && c != 127) sanitized[j++] = (char)c;
    }
    sanitized[j] = '\0';
    if (j == 0) return;

    int src_idx = -1;
    pthread_mutex_lock(&srv->lock);
    for (int i = 0; i < srv->client_count; ++i) {
        if (srv->clients[i] == src_fd) { src_idx = i; break; }
    }
    char final[1200];
    snprintf(final, sizeof(final), "[Client-%d]: %s\n", src_idx, sanitized);
    for (int i = 0; i < srv->client_count; ++i) {
        int fd = srv->clients[i];
        if (fd != src_fd) {
            send(fd, final, strlen(final), 0);
        }
    }
    pthread_mutex_unlock(&srv->lock);
}

static void remove_client(c_server_t *srv, int fd) {
    pthread_mutex_lock(&srv->lock);
    int idx = -1;
    for (int i = 0; i < srv->client_count; ++i) {
        if (srv->clients[i] == fd) { idx = i; break; }
    }
    if (idx >= 0) {
        for (int i = idx + 1; i < srv->client_count; ++i) srv->clients[i-1] = srv->clients[i];
        srv->client_count--;
    }
    pthread_mutex_unlock(&srv->lock);
}

static void *client_thread(void *arg) {
    int fd = *(int*)arg;
    c_server_t *srv = (c_server_t*)(((void**)arg)[1]);
    free(arg);

    set_rcv_timeout(fd, SOCKET_TIMEOUT_SEC);
    const char *welcome = "[Server]: Welcome!\n";
    send(fd, welcome, strlen(welcome), 0);

    char line[MAX_MSG_LEN + 2];
    size_t len = 0;
    char ch;
    while (srv->running) {
        ssize_t r = recv(fd, &ch, 1, 0);
        if (r == 0) break;
        if (r < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) continue;
            break;
        }
        if (ch == '\n') {
            line[len] = '\0';
            // trim
            char tmp[MAX_MSG_LEN + 2];
            size_t k = 0;
            for (size_t i = 0; i < len && k < sizeof(tmp)-1; ++i) {
                tmp[k++] = line[i];
            }
            tmp[k] = '\0';
            // lower and trim spaces
            for (size_t i = 0; i < k; ++i) tmp[i] = (char)tolower((unsigned char)tmp[i]);
            while (k > 0 && (tmp[k-1] == ' ' || tmp[k-1] == '\r')) tmp[--k] = '\0';
            size_t start = 0; while (start < k && tmp[start] == ' ') start++;
            if (strcmp(tmp + start, "quit") == 0) break;
            broadcast_msg(srv, line, fd);
            len = 0;
        } else if (ch != '\r') {
            if (len < MAX_MSG_LEN) {
                line[len++] = ch;
            } else {
                const char *warn = "[Server]: Message too long. Disconnecting.\n";
                send(fd, warn, strlen(warn), 0);
                break;
            }
        }
    }
    safe_close(fd);
    remove_client(srv, fd);
    return NULL;
}

static void *accept_loop(void *arg) {
    c_server_t *srv = (c_server_t*)arg;
    while (srv->running) {
        struct sockaddr_in cli;
        socklen_t clen = sizeof(cli);
        int cfd = accept(srv->srv_fd, (struct sockaddr*)&cli, &clen);
        if (cfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                usleep(50000);
                continue;
            }
            usleep(10000);
            continue;
        }
        pthread_mutex_lock(&srv->lock);
        if (srv->client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&srv->lock);
            safe_close(cfd);
            continue;
        }
        srv->clients[srv->client_count++] = cfd;
        pthread_mutex_unlock(&srv->lock);

        // pass fd and srv via heap
        void **pack = malloc(2 * sizeof(void*));
        int *fdptr = malloc(sizeof(int));
        *fdptr = cfd;
        pack[0] = fdptr;
        pack[1] = srv;
        pthread_t th;
        pthread_create(&th, NULL, client_thread, pack);
        pthread_detach(th);
    }
    return NULL;
}

static int start_server(int port, int max_clients_ignored, c_server_t *srv) {
    (void)max_clients_ignored; // using compile-time MAX_CLIENTS for simplicity
    memset(srv, 0, sizeof(*srv));
    pthread_mutex_init(&srv->lock, NULL);
    srv->srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->srv_fd < 0) return -1;
    int yes = 1;
    setsockopt(srv->srv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons((uint16_t)port);

    if (bind(srv->srv_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        safe_close(srv->srv_fd); srv->srv_fd = -1; return -1;
    }
    if (listen(srv->srv_fd, BACKLOG) != 0) {
        safe_close(srv->srv_fd); srv->srv_fd = -1; return -1;
    }
    // get actual port
    socklen_t alen = sizeof(addr);
    if (getsockname(srv->srv_fd, (struct sockaddr*)&addr, &alen) != 0) {
        safe_close(srv->srv_fd); srv->srv_fd = -1; return -1;
    }
    // set non-block accept by SO_RCVTIMEO? We'll just loop with EAGAIN
    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    setsockopt(srv->srv_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Not strictly non-blocking for accept, but timeout helps.

    srv->running = 1;
    pthread_create(&srv->accept_thread, NULL, accept_loop, srv);
    return ntohs(addr.sin_port);
}

static void stop_server(c_server_t *srv) {
    if (!srv->running) return;
    srv->running = 0;
    safe_close(srv->srv_fd);
    srv->srv_fd = -1;
    pthread_join(srv->accept_thread, NULL);
    pthread_mutex_lock(&srv->lock);
    for (int i = 0; i < srv->client_count; ++i) safe_close(srv->clients[i]);
    srv->client_count = 0;
    pthread_mutex_unlock(&srv->lock);
    pthread_mutex_destroy(&srv->lock);
}

// Test client
static void client_send_and_receive(const char *host, int port, const char *msg, int expectReads, int timeoutSec) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    struct timeval tv; tv.tv_sec = timeoutSec; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return; }
    size_t mlen = strlen(msg);
    char *sendbuf = (char*)malloc(mlen + 2);
    memcpy(sendbuf, msg, mlen);
    sendbuf[mlen] = '\n'; sendbuf[mlen+1] = '\0';
    send(fd, sendbuf, mlen + 1, 0);
    free(sendbuf);
    // read some lines
    char ch; int reads = 0; char line[2048]; size_t l = 0;
    while (reads < expectReads) {
        ssize_t r = recv(fd, &ch, 1, 0);
        if (r == 0) break;
        if (r < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR) break;
            break;
        }
        if (ch == '\n') {
            line[l] = '\0';
            // printf("Client got: %s\n", line);
            l = 0; reads++;
        } else if (ch != '\r') {
            if (l < sizeof(line)-1) line[l++] = ch;
        }
    }
    const char *quit = "quit\n";
    send(fd, quit, strlen(quit), 0);
    close(fd);
}

int main(void) {
    c_server_t server;
    int port = start_server(0, 16, &server);
    if (port <= 0) return 1;

    pthread_t clients[5];
    for (int i = 0; i < 5; ++i) {
        struct args_pack { int port; int idx; } *pack = malloc(sizeof(struct args_pack));
        pack->port = port; pack->idx = i;
        pthread_create(&clients[i], NULL, (void*(*)(void*))[](void *ap)->void* {
            struct args_pack *p = (struct args_pack*)ap;
            char msg[64];
            snprintf(msg, sizeof(msg), "Hello from C client %d", p->idx);
            client_send_and_receive("127.0.0.1", p->port, msg, 3, 3);
            free(p);
            return NULL;
        }, pack);
    }
    for (int i = 0; i < 5; ++i) pthread_join(clients[i], NULL);

    stop_server(&server);
    // printf("C server test completed.\n");
    return 0;
}