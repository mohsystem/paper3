/*
Simple multi-client chat application in C using TCP and threads.
Server accepts multiple client connections and broadcasts messages to all clients.
Each message is length-prefixed with 4 bytes (big-endian uint32).
Includes a main method that starts the server and 5 client test cases.

Build: gcc -pthread -Wall -Wextra -O2 chat.c -o chat
Run: ./chat
*/

#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define MAX_CLIENTS 64
#define MAX_MESSAGE 256
#define CLIENT_INBUF 4096
#define SERVER_BACKLOG 16

typedef struct {
    int fd;
    int id;
    uint8_t inbuf[CLIENT_INBUF];
    size_t in_len;
    int alive;
} Client;

typedef struct {
    int listen_fd;
    int control_fds[2]; // control_fds[0] read end for server thread, control_fds[1] write end for stop
    Client clients[MAX_CLIENTS];
    int running;
    uint16_t port;
    pthread_t thread;
} ChatServer;

typedef struct {
    uint16_t port;
    char message[MAX_MESSAGE + 1];
    int id;
} ClientArgs;

static int set_cloexec(int fd) {
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) return -1;
    return 0;
}

static int set_nonblock(int fd, int enable) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (enable) flags |= O_NONBLOCK;
    else flags &= ~O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) return -1;
    return 0;
}

static int set_timeouts(int fd, int send_ms, int recv_ms) {
    struct timeval tv;
    tv.tv_sec = send_ms / 1000;
    tv.tv_usec = (send_ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) return -1;
    tv.tv_sec = recv_ms / 1000;
    tv.tv_usec = (recv_ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) return -1;
#ifdef SO_NOSIGPIPE
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one)); // best effort
#endif
    return 0;
}

static int create_listen_socket(uint16_t port, uint16_t* out_port) {
    int fd = -1;
    int rc = -1;
    int yes = 1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) goto fail;
    if (set_cloexec(fd) != 0) goto fail;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) goto fail;

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) goto fail;
    if (listen(fd, SERVER_BACKLOG) < 0) goto fail;

    struct sockaddr_in got;
    socklen_t slen = sizeof(got);
    if (getsockname(fd, (struct sockaddr*)&got, &slen) < 0) goto fail;
    if (out_port) *out_port = ntohs(got.sin_port);

    rc = fd;
    return rc;

fail:
    if (fd >= 0) close(fd);
    return -1;
}

static int send_all_with_timeout(int fd, const uint8_t* buf, size_t len, int timeout_ms) {
    size_t off = 0;
    int elapsed = 0;
    const int step = 200; // ms
    while (off < len) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLOUT;
        int wait = (timeout_ms - elapsed) > step ? step : (timeout_ms - elapsed);
        if (wait < 0) wait = 0;
        int pr = poll(&pfd, 1, wait);
        if (pr < 0) {
            if (errno == EINTR) continue;
            return -1;
        } else if (pr == 0) {
            elapsed += wait;
            if (elapsed >= timeout_ms) return -1;
            continue;
        }
        ssize_t n = send(fd, buf + off, len - off, MSG_NOSIGNAL);
        if (n < 0) {
            if (errno == EINTR) continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // let poll wait again
                continue;
            }
            return -1;
        } else if (n == 0) {
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

static int send_frame(int fd, const uint8_t* payload, size_t len, int timeout_ms) {
    if (len > MAX_MESSAGE) return -1;
    uint8_t frame[4 + MAX_MESSAGE];
    uint32_t nlen = htonl((uint32_t)len);
    memcpy(frame, &nlen, 4);
    if (len > 0) memcpy(frame + 4, payload, len);
    return send_all_with_timeout(fd, frame, 4 + len, timeout_ms);
}

static void close_client(Client* c) {
    if (c == NULL) return;
    if (c->alive && c->fd >= 0) {
        shutdown(c->fd, SHUT_RDWR);
        close(c->fd);
    }
    c->fd = -1;
    c->id = -1;
    c->in_len = 0;
    c->alive = 0;
    memset(c->inbuf, 0, sizeof(c->inbuf));
}

static int add_client(ChatServer* srv, int fd, int id) {
    if (!srv) return -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!srv->clients[i].alive) {
            srv->clients[i].fd = fd;
            srv->clients[i].id = id;
            srv->clients[i].in_len = 0;
            srv->clients[i].alive = 1;
            return 0;
        }
    }
    return -1;
}

static int broadcast(ChatServer* srv, const uint8_t* payload, size_t len) {
    if (!srv || !payload || len > MAX_MESSAGE) return -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->clients[i].alive) {
            if (send_frame(srv->clients[i].fd, payload, len, 1000) != 0) {
                // Close misbehaving client
                close_client(&srv->clients[i]);
            }
        }
    }
    return 0;
}

static void handle_client_read(ChatServer* srv, Client* c) {
    if (!srv || !c || !c->alive) return;
    uint8_t buf[512];
    ssize_t n = recv(c->fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        close_client(c);
        return;
    }
    if ((size_t)n > sizeof(buf)) return; // defensive

    if (c->in_len + (size_t)n > sizeof(c->inbuf)) {
        // overflow risk, drop client
        close_client(c);
        return;
    }
    memcpy(c->inbuf + c->in_len, buf, (size_t)n);
    c->in_len += (size_t)n;

    // Parse frames
    size_t offset = 0;
    while (c->in_len - offset >= 4) {
        uint32_t be_len = 0;
        memcpy(&be_len, c->inbuf + offset, 4);
        uint32_t mlen = ntohl(be_len);
        if (mlen > MAX_MESSAGE) {
            close_client(c);
            return;
        }
        if (c->in_len - offset < 4u + mlen) {
            break; // wait for more
        }
        const uint8_t* payload = c->inbuf + offset + 4;
        // Broadcast to all clients (including sender)
        broadcast(srv, payload, mlen);
        offset += 4u + mlen;
    }
    if (offset > 0) {
        size_t remaining = c->in_len - offset;
        memmove(c->inbuf, c->inbuf + offset, remaining);
        c->in_len = remaining;
    }
}

static void* server_thread_main(void* arg) {
    ChatServer* srv = (ChatServer*)arg;
    if (!srv) return NULL;
    int next_id = 1;
    srv->running = 1;

    while (srv->running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = -1;

        if (srv->listen_fd >= 0) {
            FD_SET(srv->listen_fd, &rfds);
            if (srv->listen_fd > maxfd) maxfd = srv->listen_fd;
        }
        if (srv->control_fds[0] >= 0) {
            FD_SET(srv->control_fds[0], &rfds);
            if (srv->control_fds[0] > maxfd) maxfd = srv->control_fds[0];
        }
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (srv->clients[i].alive && srv->clients[i].fd >= 0) {
                FD_SET(srv->clients[i].fd, &rfds);
                if (srv->clients[i].fd > maxfd) maxfd = srv->clients[i].fd;
            }
        }

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int sel = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (sel < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (sel == 0) {
            continue; // timeout, loop again
        }

        // Control signal
        if (srv->control_fds[0] >= 0 && FD_ISSET(srv->control_fds[0], &rfds)) {
            uint8_t ch;
            if (read(srv->control_fds[0], &ch, 1) > 0) {
                srv->running = 0;
            }
        }

        // Accept new connections
        if (srv->listen_fd >= 0 && FD_ISSET(srv->listen_fd, &rfds)) {
            for (;;) {
                struct sockaddr_in caddr;
                socklen_t clen = sizeof(caddr);
                int cfd = accept(srv->listen_fd, (struct sockaddr*)&caddr, &clen);
                if (cfd < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    if (errno == EINTR) continue;
                    break;
                }
                (void)set_cloexec(cfd);
                (void)set_timeouts(cfd, 1000, 5000);
                if (add_client(srv, cfd, next_id) != 0) {
                    // server full
                    close(cfd);
                } else {
                    next_id++;
                }
            }
        }

        // Handle client reads
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (srv->clients[i].alive && srv->clients[i].fd >= 0 && FD_ISSET(srv->clients[i].fd, &rfds)) {
                handle_client_read(srv, &srv->clients[i]);
            }
        }
    }

    // Cleanup clients
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (srv->clients[i].alive) {
            close_client(&srv->clients[i]);
        }
    }
    if (srv->listen_fd >= 0) {
        close(srv->listen_fd);
        srv->listen_fd = -1;
    }
    if (srv->control_fds[0] >= 0) {
        close(srv->control_fds[0]);
        srv->control_fds[0] = -1;
    }
    if (srv->control_fds[1] >= 0) {
        close(srv->control_fds[1]);
        srv->control_fds[1] = -1;
    }
    return NULL;
}

static int start_chat_server(ChatServer* srv, uint16_t port) {
    if (!srv) return -1;
    memset(srv, 0, sizeof(*srv));
    for (int i = 0; i < MAX_CLIENTS; i++) {
        srv->clients[i].fd = -1;
        srv->clients[i].alive = 0;
        srv->clients[i].id = -1;
        srv->clients[i].in_len = 0;
    }
    srv->listen_fd = -1;
    srv->control_fds[0] = -1;
    srv->control_fds[1] = -1;
    srv->running = 0;
    srv->port = 0;

    // Ignore SIGPIPE to avoid process termination on send to closed sockets
#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

    int lfd = create_listen_socket(port, &srv->port);
    if (lfd < 0) return -1;
    srv->listen_fd = lfd;
    if (set_nonblock(srv->listen_fd, 1) != 0) {
        close(srv->listen_fd);
        return -1;
    }

    if (pipe(srv->control_fds) != 0) {
        close(srv->listen_fd);
        return -1;
    }
    set_cloexec(srv->control_fds[0]);
    set_cloexec(srv->control_fds[1]);

    if (pthread_create(&srv->thread, NULL, server_thread_main, srv) != 0) {
        close(srv->listen_fd);
        close(srv->control_fds[0]);
        close(srv->control_fds[1]);
        srv->listen_fd = -1;
        srv->control_fds[0] = srv->control_fds[1] = -1;
        return -1;
    }
    // Wait a short time for server to start listening
    struct timespec ts = {0, 100 * 1000000}; // 100ms
    nanosleep(&ts, NULL);
    return 0;
}

static int stop_chat_server(ChatServer* srv) {
    if (!srv) return -1;
    uint8_t ch = 1;
    if (srv->control_fds[1] >= 0) {
        if (write(srv->control_fds[1], &ch, 1) < 0) {
            return -1;
        }
        return 0;
    }
    return -1;
}

static int join_chat_server(ChatServer* srv) {
    if (!srv) return -1;
    void* ret = NULL;
    if (pthread_join(srv->thread, &ret) != 0) return -1;
    return 0;
}

static int connect_with_timeout(const char* host, uint16_t port, int timeout_ms) {
    if (!host) return -1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    set_cloexec(fd);
    set_timeouts(fd, timeout_ms, timeout_ms);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    addr.sin_port = htons(port);

    // Non-blocking connect with poll for timeout
    if (set_nonblock(fd, 1) != 0) {
        close(fd);
        return -1;
    }
    int rc = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (rc < 0 && errno != EINPROGRESS) {
        close(fd);
        return -1;
    }
    if (rc != 0) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLOUT;
        rc = poll(&pfd, 1, timeout_ms);
        if (rc <= 0) {
            close(fd);
            return -1;
        }
        int err = 0;
        socklen_t len = sizeof(err);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
            close(fd);
            return -1;
        }
    }
    (void)set_nonblock(fd, 0);
    return fd;
}

static void parse_and_print_frames(uint8_t* buf, size_t* in_len) {
    size_t offset = 0;
    while (*in_len - offset >= 4) {
        uint32_t be_len = 0;
        memcpy(&be_len, buf + offset, 4);
        uint32_t mlen = ntohl(be_len);
        if (mlen > MAX_MESSAGE) {
            // invalid, drop everything
            *in_len = 0;
            return;
        }
        if (*in_len - offset < 4u + mlen) break;
        uint8_t* payload = buf + offset + 4;
        // Ensure null-terminated for printing
        char out[MAX_MESSAGE + 1];
        size_t cpy = mlen <= MAX_MESSAGE ? mlen : MAX_MESSAGE;
        memcpy(out, payload, cpy);
        out[cpy] = '\0';
        printf("Client received: %s\n", out);
        offset += 4u + mlen;
    }
    if (offset > 0) {
        size_t remaining = *in_len - offset;
        memmove(buf, buf + offset, remaining);
        *in_len = remaining;
    }
}

static void* client_thread_main(void* arg) {
    ClientArgs* ca = (ClientArgs*)arg;
    if (!ca) return NULL;
    int fd = connect_with_timeout("127.0.0.1", ca->port, 3000);
    if (fd < 0) {
        fprintf(stderr, "Client %d failed to connect\n", ca->id);
        return NULL;
    }
    // Send message
    size_t msglen = strnlen(ca->message, MAX_MESSAGE);
    if (send_frame(fd, (const uint8_t*)ca->message, msglen, 2000) != 0) {
        fprintf(stderr, "Client %d failed to send\n", ca->id);
        close(fd);
        return NULL;
    }

    // Receive broadcasts for up to 2 seconds
    uint8_t inbuf[CLIENT_INBUF];
    size_t in_len = 0;
    int elapsed = 0;
    const int step = 200;
    while (elapsed < 2000) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        int pr = poll(&pfd, 1, step);
        if (pr < 0) {
            if (errno == EINTR) continue;
            break;
        } else if (pr == 0) {
            elapsed += step;
            continue;
        }
        if (pfd.revents & POLLIN) {
            ssize_t n = recv(fd, inbuf + in_len, sizeof(inbuf) - in_len, 0);
            if (n <= 0) break;
            in_len += (size_t)n;
            if (in_len > sizeof(inbuf)) break;
            parse_and_print_frames(inbuf, &in_len);
        }
    }

    shutdown(fd, SHUT_RDWR);
    close(fd);
    return NULL;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    ChatServer server;
    if (start_chat_server(&server, 0) != 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    printf("Server listening on port %u\n", (unsigned)server.port);

    // Prepare 5 test case clients
    const char* tests[5] = {
        "Alpha: Hello World!",
        "Bravo: Secure Chat",
        "Charlie: Broadcasting",
        "Delta: Multi-client",
        "Echo: Final message"
    };
    pthread_t cthreads[5];
    ClientArgs args[5];
    for (int i = 0; i < 5; i++) {
        memset(&args[i], 0, sizeof(args[i]));
        args[i].port = server.port;
        args[i].id = i + 1;
        // Copy message safely
        size_t len = strnlen(tests[i], MAX_MESSAGE);
        memcpy(args[i].message, tests[i], len);
        args[i].message[len] = '\0';
        if (pthread_create(&cthreads[i], NULL, client_thread_main, &args[i]) != 0) {
            fprintf(stderr, "Failed to create client thread %d\n", i + 1);
        }
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(cthreads[i], NULL);
    }

    // Stop server
    stop_chat_server(&server);
    join_chat_server(&server);

    printf("Chat application test completed.\n");
    return 0;
}