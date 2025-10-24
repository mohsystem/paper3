#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple dynamic array for client sockets
typedef struct {
    int fd;
    char name[64];
    pthread_t thread;
} client_t;

typedef struct {
    int server_fd;
    volatile int running;
    client_t* clients;
    size_t clients_len;
    size_t clients_cap;
    pthread_mutex_t mx;
    pthread_t accept_thread;
    int next_id;
} chat_server_t;

static void cs_init(chat_server_t* cs) {
    cs->server_fd = -1;
    cs->running = 0;
    cs->clients = NULL;
    cs->clients_len = 0;
    cs->clients_cap = 0;
    pthread_mutex_init(&cs->mx, NULL);
    cs->next_id = 1;
}

static void cs_free(chat_server_t* cs) {
    pthread_mutex_destroy(&cs->mx);
    free(cs->clients);
}

static int send_all(int fd, const char* data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, data + off, len - off, 0);
        if (n <= 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

static void sanitize(const char* in, char* out, size_t out_sz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0' && oi + 1 < out_sz && i < 1024; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '\n' || c == '\r') continue;
        if (c >= 32 && c <= 126) {
            out[oi++] = (char)c;
        } else {
            out[oi++] = '?';
        }
    }
    out[oi] = '\0';
}

static void broadcast(chat_server_t* cs, const char* msg) {
    pthread_mutex_lock(&cs->mx);
    // copy fds
    size_t n = cs->clients_len;
    int* fds = (int*)malloc(sizeof(int) * n);
    if (!fds) { pthread_mutex_unlock(&cs->mx); return; }
    for (size_t i = 0; i < n; i++) fds[i] = cs->clients[i].fd;
    pthread_mutex_unlock(&cs->mx);

    size_t mlen = strlen(msg);
    char* line = (char*)malloc(mlen + 2);
    if (!line) { free(fds); return; }
    memcpy(line, msg, mlen);
    line[mlen] = '\n';
    line[mlen + 1] = '\0';

    for (size_t i = 0; i < n; i++) {
        if (send_all(fds[i], line, mlen + 1) != 0) {
            // ignore failure here; cleanup on read side
        }
    }
    free(line);
    free(fds);
}

typedef struct {
    chat_server_t* cs;
    int fd;
    char name[64];
} client_arg_t;

static int recv_line(int fd, char* out, size_t out_sz) {
    size_t oi = 0;
    char buf[256];
    for (;;) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n == 0) return 0; // closed
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        for (ssize_t i = 0; i < n; i++) {
            char c = buf[i];
            if (c == '\n') {
                out[oi] = '\0';
                return 1;
            } else if (c != '\r') {
                if (oi + 1 < out_sz) {
                    out[oi++] = c;
                }
            }
        }
        if (oi + 1 >= out_sz) {
            out[oi] = '\0';
            return 1;
        }
    }
}

static void* client_thread(void* p) {
    client_arg_t* arg = (client_arg_t*)p;
    chat_server_t* cs = arg->cs;
    int fd = arg->fd;
    char name[64];
    snprintf(name, sizeof(name), "%s", arg->name);
    free(arg);

    struct timeval tv;
    tv.tv_sec = 15;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char line[2048];
    while (cs->running) {
        int r = recv_line(fd, line, sizeof(line));
        if (r <= 0) break;
        char clean[1200];
        sanitize(line, clean, sizeof(clean));
        if (strcmp(clean, "/quit") == 0) break;
        if (clean[0] != '\0') {
            char msg[1400];
            snprintf(msg, sizeof(msg), "%s: %s", name, clean);
            broadcast(cs, msg);
        }
    }

    // Remove client from list
    pthread_mutex_lock(&cs->mx);
    for (size_t i = 0; i < cs->clients_len; i++) {
        if (cs->clients[i].fd == fd) {
            // detach thread record
            // actual pthread is this thread; main list only stores fd/thread for reference
            // Shift remaining
            for (size_t j = i + 1; j < cs->clients_len; j++) {
                cs->clients[j - 1] = cs->clients[j];
            }
            cs->clients_len--;
            break;
        }
    }
    pthread_mutex_unlock(&cs->mx);

    char leftmsg[128];
    snprintf(leftmsg, sizeof(leftmsg), "[server] %s left", name);
    broadcast(cs, leftmsg);

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return NULL;
}

static void* accept_thread(void* p) {
    chat_server_t* cs = (chat_server_t*)p;
    while (cs->running) {
        struct sockaddr_in cli;
        socklen_t cl = sizeof(cli);
        int cfd = accept(cs->server_fd, (struct sockaddr*)&cli, &cl);
        if (cfd < 0) {
            if (!cs->running) break;
            if (errno == EINTR) continue;
            usleep(50000);
            continue;
        }
        int id = __sync_fetch_and_add(&cs->next_id, 1);
        client_t rec;
        rec.fd = cfd;
        snprintf(rec.name, sizeof(rec.name), "client-%d", id);

        pthread_mutex_lock(&cs->mx);
        if (cs->clients_len == cs->clients_cap) {
            size_t ncap = cs->clients_cap == 0 ? 8 : cs->clients_cap * 2;
            client_t* nbuf = (client_t*)realloc(cs->clients, ncap * sizeof(client_t));
            if (nbuf) {
                cs->clients = nbuf;
                cs->clients_cap = ncap;
            }
        }
        if (cs->clients_len < cs->clients_cap) {
            cs->clients[cs->clients_len++] = rec;
        }
        pthread_mutex_unlock(&cs->mx);

        // announce join
        char msg[128];
        snprintf(msg, sizeof(msg), "[server] %s joined", rec.name);
        broadcast(cs, msg);

        client_arg_t* carg = (client_arg_t*)malloc(sizeof(client_arg_t));
        if (!carg) { close(cfd); continue; }
        carg->cs = cs;
        carg->fd = cfd;
        snprintf(carg->name, sizeof(carg->name), "%s", rec.name);
        pthread_t th;
        if (pthread_create(&th, NULL, client_thread, carg) == 0) {
            pthread_detach(th);
        } else {
            close(cfd);
            free(carg);
        }
    }
    return NULL;
}

static int cs_start(chat_server_t* cs, uint16_t port) {
    if (cs->running) return -1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        close(fd);
        return -1;
    }
    if (listen(fd, 20) < 0) {
        close(fd);
        return -1;
    }
    cs->server_fd = fd;
    cs->running = 1;
    if (pthread_create(&cs->accept_thread, NULL, accept_thread, cs) != 0) {
        close(fd);
        cs->running = 0;
        return -1;
    }
    return 0;
}

static void cs_stop(chat_server_t* cs) {
    if (!cs->running) return;
    cs->running = 0;
    if (cs->server_fd >= 0) {
        shutdown(cs->server_fd, SHUT_RDWR);
        close(cs->server_fd);
        cs->server_fd = -1;
    }
    pthread_join(cs->accept_thread, NULL);
    // Close all clients
    pthread_mutex_lock(&cs->mx);
    for (size_t i = 0; i < cs->clients_len; i++) {
        shutdown(cs->clients[i].fd, SHUT_RDWR);
        close(cs->clients[i].fd);
    }
    cs->clients_len = 0;
    pthread_mutex_unlock(&cs->mx);
}

static uint16_t cs_port(chat_server_t* cs) {
    if (cs->server_fd < 0) return 0;
    struct sockaddr_in sa;
    socklen_t sl = sizeof(sa);
    if (getsockname(cs->server_fd, (struct sockaddr*)&sa, &sl) == 0) {
        return ntohs(sa.sin_port);
    }
    return 0;
}

// Simple barrier for tests
typedef struct {
    pthread_mutex_t mx;
    pthread_cond_t cv;
    int threshold;
    int count;
    int generation;
} barrier_t;

static void barrier_init(barrier_t* b, int count) {
    pthread_mutex_init(&b->mx, NULL);
    pthread_cond_init(&b->cv, NULL);
    b->threshold = count;
    b->count = count;
    b->generation = 0;
}
static void barrier_wait(barrier_t* b) {
    pthread_mutex_lock(&b->mx);
    int gen = b->generation;
    if (--b->count == 0) {
        b->generation++;
        b->count = b->threshold;
        pthread_cond_broadcast(&b->cv);
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5;
        while (gen == b->generation) {
            if (pthread_cond_timedwait(&b->cv, &b->mx, &ts) == ETIMEDOUT) break;
        }
    }
    pthread_mutex_unlock(&b->mx);
}

typedef struct {
    int id;
    char host[16];
    uint16_t port;
    barrier_t* bar;
    int expected;
    int received;
} test_client_t;

static int connect_timeout(const char* host, uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, host, &sa.sin_addr);
    sa.sin_port = htons(port);
    if (connect(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static int recv_line_nonblock(int fd, char* out, size_t out_sz, int ms_timeout) {
    size_t oi = 0;
    struct timeval start, now;
    gettimeofday(&start, NULL);
    for (;;) {
        char c;
        ssize_t n = recv(fd, &c, 1, MSG_DONTWAIT);
        if (n == 1) {
            if (c == '\n') { out[oi] = '\0'; return 1; }
            if (c != '\r' && oi + 1 < out_sz) out[oi++] = c;
        } else if (n == 0) {
            return 0;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) return -1;
            usleep(5000);
        }
        gettimeofday(&now, NULL);
        int elapsed = (int)((now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000);
        if (elapsed >= ms_timeout) {
            out[oi] = '\0';
            return oi > 0 ? 1 : -1;
        }
    }
}

static void* test_client_thread(void* p) {
    test_client_t* tc = (test_client_t*)p;
    int fd = connect_timeout(tc->host, tc->port);
    if (fd < 0) return NULL;

    barrier_wait(tc->bar);

    char msg[128];
    snprintf(msg, sizeof(msg), "Hello from test client %d\n", tc->id);
    send_all(fd, msg, strlen(msg));

    int count = 0;
    char line[1024];
    int deadline_ms = 5000;
    int elapsed = 0;
    while (count < tc->expected && elapsed < deadline_ms) {
        int r = recv_line_nonblock(fd, line, sizeof(line), 200);
        if (r <= 0) {
            elapsed += 200;
            continue;
        }
        count++;
        elapsed += 0;
    }
    tc->received = count;
    send_all(fd, "/quit\n", 6);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    return NULL;
}

int main(void) {
    chat_server_t cs;
    cs_init(&cs);
    if (cs_start(&cs, 0) != 0) {
        fprintf(stderr, "Failed to start server\n");
        cs_free(&cs);
        return 1;
    }
    uint16_t port = cs_port(&cs);
    const int N = 5;
    barrier_t bar;
    barrier_init(&bar, N);

    test_client_t tcs[N];
    pthread_t th[N];
    for (int i = 0; i < N; i++) {
        tcs[i].id = i + 1;
        snprintf(tcs[i].host, sizeof(tcs[i].host), "127.0.0.1");
        tcs[i].port = port;
        tcs[i].bar = &bar;
        tcs[i].expected = N;
        tcs[i].received = 0;
        pthread_create(&th[i], NULL, test_client_thread, &tcs[i]);
    }
    for (int i = 0; i < N; i++) {
        pthread_join(th[i], NULL);
    }
    for (int i = 0; i < N; i++) {
        printf("Client %d received %d lines\n", i + 1, tcs[i].received);
    }
    cs_stop(&cs);
    printf("Server stopped.\n");
    cs_free(&cs);
    return 0;
}