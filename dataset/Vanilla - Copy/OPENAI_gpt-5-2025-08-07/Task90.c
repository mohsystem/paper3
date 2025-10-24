#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    int *data;
    size_t size;
    size_t cap;
} IntVec;

static void ivec_init(IntVec *v) { v->data = NULL; v->size = 0; v->cap = 0; }
static void ivec_push(IntVec *v, int val) {
    if (v->size == v->cap) {
        size_t ncap = v->cap ? v->cap * 2 : 8;
        int *nd = (int*)realloc(v->data, ncap * sizeof(int));
        if (!nd) return;
        v->data = nd; v->cap = ncap;
    }
    v->data[v->size++] = val;
}
static void ivec_remove(IntVec *v, int val) {
    for (size_t i = 0; i < v->size; ++i) {
        if (v->data[i] == val) {
            v->data[i] = v->data[v->size - 1];
            v->size--;
            return;
        }
    }
}
static void ivec_free(IntVec *v) { free(v->data); v->data = NULL; v->size = v->cap = 0; }

typedef struct ChatServer {
    int port;
    int server_fd;
    volatile sig_atomic_t running;
    pthread_t accept_thread;
    pthread_mutex_t clients_mtx;
    IntVec clients;
    int next_id;
} ChatServer;

typedef struct {
    ChatServer *srv;
    int cfd;
    int id;
} ClientArgs;

static void server_broadcast(ChatServer *srv, const char *msg, size_t len) {
    pthread_mutex_lock(&srv->clients_mtx);
    for (size_t i = 0; i < srv->clients.size; ) {
        int c = srv->clients.data[i];
        ssize_t n = send(c, msg, len, MSG_NOSIGNAL);
        if (n < 0) {
            shutdown(c, SHUT_RDWR);
            close(c);
            srv->clients.data[i] = srv->clients.data[srv->clients.size - 1];
            srv->clients.size--;
        } else {
            i++;
        }
    }
    pthread_mutex_unlock(&srv->clients_mtx);
}

static void* client_thread(void *arg) {
    ClientArgs *ca = (ClientArgs*)arg;
    ChatServer *srv = ca->srv;
    int cfd = ca->cfd;
    int id = ca->id;
    free(ca);
    char name[64];
    snprintf(name, sizeof(name), "Client-%d", id);
    char buf[4096];

    while (srv->running) {
        ssize_t n = recv(cfd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        // build message "Client-id: <payload>"
        size_t hdr_len = snprintf(NULL, 0, "%s: ", name);
        char *msg = (char*)malloc(hdr_len + n + 1);
        if (!msg) break;
        snprintf(msg, hdr_len + 1, "%s: ", name);
        memcpy(msg + hdr_len, buf, (size_t)n);
        msg[hdr_len + n] = '\0';
        server_broadcast(srv, msg, hdr_len + (size_t)n);
        free(msg);
    }

    pthread_mutex_lock(&srv->clients_mtx);
    ivec_remove(&srv->clients, cfd);
    pthread_mutex_unlock(&srv->clients_mtx);
    shutdown(cfd, SHUT_RDWR);
    close(cfd);
    return NULL;
}

static void* accept_loop(void *arg) {
    ChatServer *srv = (ChatServer*)arg;
    while (srv->running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(srv->server_fd, &rfds);
        struct timeval tv; tv.tv_sec = 0; tv.tv_usec = 200000;
        int r = select(srv->server_fd + 1, &rfds, NULL, NULL, &tv);
        if (r <= 0) continue;

        struct sockaddr_in cli;
        socklen_t cl = sizeof(cli);
        int cfd = accept(srv->server_fd, (struct sockaddr*)&cli, &cl);
        if (cfd < 0) continue;

        pthread_mutex_lock(&srv->clients_mtx);
        ivec_push(&srv->clients, cfd);
        int id = srv->next_id++;
        pthread_mutex_unlock(&srv->clients_mtx);

        // send welcome
        char welcome[128];
        int wl = snprintf(welcome, sizeof(welcome), "Welcome Client-%d\n", id);
        send(cfd, welcome, (size_t)wl, MSG_NOSIGNAL);

        ClientArgs *ca = (ClientArgs*)malloc(sizeof(ClientArgs));
        if (!ca) { close(cfd); continue; }
        ca->srv = srv; ca->cfd = cfd; ca->id = id;
        pthread_t th;
        pthread_create(&th, NULL, client_thread, ca);
        pthread_detach(th);
    }
    return NULL;
}

ChatServer* start_server(int port) {
    ChatServer *srv = (ChatServer*)calloc(1, sizeof(ChatServer));
    if (!srv) return NULL;
    srv->port = port;
    srv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->server_fd < 0) { free(srv); return NULL; }
    int opt = 1;
    setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(srv->server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(srv->server_fd); free(srv); return NULL;
    }
    if (listen(srv->server_fd, 128) < 0) {
        close(srv->server_fd); free(srv); return NULL;
    }
    srv->running = 1;
    pthread_mutex_init(&srv->clients_mtx, NULL);
    ivec_init(&srv->clients);
    srv->next_id = 1;
    pthread_create(&srv->accept_thread, NULL, accept_loop, srv);
    return srv;
}

void stop_server(ChatServer *srv) {
    if (!srv) return;
    srv->running = 0;
    shutdown(srv->server_fd, SHUT_RDWR);
    close(srv->server_fd);
    pthread_join(srv->accept_thread, NULL);
    pthread_mutex_lock(&srv->clients_mtx);
    for (size_t i = 0; i < srv->clients.size; ++i) {
        shutdown(srv->clients.data[i], SHUT_RDWR);
        close(srv->clients.data[i]);
    }
    ivec_free(&srv->clients);
    pthread_mutex_unlock(&srv->clients_mtx);
    pthread_mutex_destroy(&srv->clients_mtx);
    free(srv);
}

// Test client
typedef struct {
    char name[64];
    char msg[256];
    int port;
} ClientSimArgs;

static void* client_sim(void *arg) {
    ClientSimArgs *a = (ClientSimArgs*)arg;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return NULL; }
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)a->port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect"); close(fd); return NULL;
    }
    // recv welcome with small timeout using select
    fd_set rfds; FD_ZERO(&rfds); FD_SET(fd, &rfds);
    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    if (select(fd + 1, &rfds, NULL, NULL, &tv) > 0) {
        char buf[512]; ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n > 0) { buf[n] = 0; printf("%s recv: %s", a->name, buf); }
    }
    char line[512];
    snprintf(line, sizeof(line), "%s\n", a->msg);
    send(fd, line, strlen(line), 0);

    FD_ZERO(&rfds); FD_SET(fd, &rfds);
    tv.tv_sec = 0; tv.tv_usec = 500000;
    if (select(fd + 1, &rfds, NULL, NULL, &tv) > 0) {
        char buf[1024]; ssize_t n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n > 0) { buf[n] = 0; printf("%s recv: %s", a->name, buf); }
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    return NULL;
}

int main(void) {
    int port = 50593;
    ChatServer *srv = start_server(port);
    if (!srv) {
        printf("Failed to start server\n");
        return 1;
    }

    pthread_t threads[5];
    ClientSimArgs args[5];
    for (int i = 0; i < 5; ++i) {
        snprintf(args[i].name, sizeof(args[i].name), "TClient-%d", i + 1);
        snprintf(args[i].msg, sizeof(args[i].msg), "Hello from TClient-%d", i + 1);
        args[i].port = port;
        pthread_create(&threads[i], NULL, client_sim, &args[i]);
    }
    for (int i = 0; i < 5; ++i) pthread_join(threads[i], NULL);

    stop_server(srv);
    printf("Server stopped.\n");
    return 0;
}