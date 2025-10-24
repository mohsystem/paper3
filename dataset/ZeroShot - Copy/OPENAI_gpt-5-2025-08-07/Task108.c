#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_VALUE 256
#define PORT 50508

typedef struct KVNode {
    char key[33];
    char val[MAX_VALUE + 1];
    struct KVNode *next;
} KVNode;

typedef struct {
    int listen_fd;
    char secret[256];
    volatile bool running;
    int counter;
    KVNode *kv_head;
    pthread_mutex_t lock;
} ServerState;

static bool const_time_eq(const char *a, const char *b) {
    if (!a || !b) return false;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) {
        volatile unsigned int diff = (unsigned int)(la ^ lb);
        size_t m = la < lb ? la : lb;
        for (size_t i = 0; i < m; i++) diff |= (unsigned int)((unsigned char)a[i] ^ (unsigned char)b[i]);
        (void)diff;
        return false;
    }
    volatile unsigned int diff = 0;
    for (size_t i = 0; i < la; i++) diff |= (unsigned int)((unsigned char)a[i] ^ (unsigned char)b[i]);
    return diff == 0;
}

static bool is_key_valid(const char *k) {
    if (!k) return false;
    size_t len = strlen(k);
    if (len == 0 || len > 32) return false;
    for (size_t i = 0; i < len; i++) {
        char c = k[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-'))
            return false;
    }
    return true;
}

static bool write_line(int fd, const char *s) {
    size_t len = strlen(s);
    size_t total = len + 1;
    char *buf = (char *)malloc(total + 1);
    if (!buf) return false;
    memcpy(buf, s, len);
    buf[len] = '\n';
    buf[len + 1] = '\0';
    size_t left = len + 1;
    char *p = buf;
    while (left > 0) {
        ssize_t n = send(fd, p, left, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(buf);
            return false;
        }
        p += n;
        left -= (size_t)n;
    }
    free(buf);
    return true;
}

static bool read_line_limited(int fd, char *out, size_t out_sz, int timeout_sec) {
    if (out_sz == 0) return false;
    size_t idx = 0;
    time_t start = time(NULL);
    while (1) {
        time_t now = time(NULL);
        if (now - start >= timeout_sec) return false;
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int r = select(fd + 1, &rfds, NULL, NULL, &tv);
        if (r < 0) return false;
        if (r == 0) continue;
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n <= 0) return false;
        if (ch == '\n') break;
        if (ch != '\r') {
            if (idx + 1 >= out_sz) return false;
            out[idx++] = ch;
        }
    }
    // trim trailing spaces
    while (idx > 0 && (out[idx - 1] == ' ' || out[idx - 1] == '\t')) idx--;
    out[idx] = '\0';
    return true;
}

static void kv_put(ServerState *st, const char *key, const char *val) {
    KVNode *prev = NULL, *cur = st->kv_head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            strncpy(cur->val, val, MAX_VALUE);
            cur->val[MAX_VALUE] = '\0';
            return;
        }
        prev = cur;
        cur = cur->next;
    }
    KVNode *node = (KVNode *)calloc(1, sizeof(KVNode));
    if (!node) return;
    strncpy(node->key, key, 32);
    node->key[32] = '\0';
    strncpy(node->val, val, MAX_VALUE);
    node->val[MAX_VALUE] = '\0';
    node->next = NULL;
    if (prev) prev->next = node;
    else st->kv_head = node;
}

static bool kv_get(ServerState *st, const char *key, char *out, size_t out_sz) {
    KVNode *cur = st->kv_head;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            strncpy(out, cur->val, out_sz - 1);
            out[out_sz - 1] = '\0';
            return true;
        }
        cur = cur->next;
    }
    return false;
}

static void kv_del(ServerState *st, const char *key) {
    KVNode *cur = st->kv_head, *prev = NULL;
    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->next = cur->next;
            else st->kv_head = cur->next;
            free(cur);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static void *client_thread(void *arg) {
    int cfd = *(int *)arg;
    free(arg);
    ServerState *st = NULL; // We'll pass via pthread-specific key? Simpler: use global pointer
    extern ServerState *g_server_state;
    st = g_server_state;
    write_line(cfd, "WELCOME");
    char line[MAX_LINE + 1];
    if (!read_line_limited(cfd, line, sizeof(line), 10)) {
        write_line(cfd, "ERR AUTH");
        close(cfd);
        return NULL;
    }
    if (strncmp(line, "TOKEN ", 6) != 0) {
        write_line(cfd, "ERR AUTH");
        close(cfd);
        return NULL;
    }
    const char *tok = line + 6;
    if (!const_time_eq(tok, st->secret)) {
        write_line(cfd, "ERR AUTH");
        close(cfd);
        return NULL;
    }
    write_line(cfd, "OK AUTH");
    while (1) {
        if (!read_line_limited(cfd, line, sizeof(line), 15)) break;
        if (line[0] == '\0') continue;
        char cmd[16] = {0};
        char a[64] = {0};
        char b[MAX_VALUE + 1] = {0};
        // Parse conservatively: commands are uppercase; tokenize by space
        {
            // Uppercase cmd safely
            size_t i = 0;
            while (line[i] && i < sizeof(cmd) - 1 && line[i] != ' ') {
                char c = line[i];
                if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
                cmd[i++] = c;
            }
            cmd[i] = '\0';
        }
        if (strcmp(cmd, "QUIT") == 0) {
            write_line(cfd, "BYE");
            break;
        } else if (strcmp(cmd, "PING") == 0) {
            write_line(cfd, "PONG");
        } else if (strcmp(cmd, "GETCNT") == 0) {
            pthread_mutex_lock(&st->lock);
            int v = st->counter;
            pthread_mutex_unlock(&st->lock);
            char resp[64];
            snprintf(resp, sizeof(resp), "COUNTER %d", v);
            write_line(cfd, resp);
        } else if (strcmp(cmd, "INCR") == 0) {
            int n;
            if (sscanf(line, "%*s %63s", a) != 1) { write_line(cfd, "ERR ARG"); continue; }
            n = atoi(a);
            if (n < 0 || n > 1000000) { write_line(cfd, "ERR RANGE"); continue; }
            pthread_mutex_lock(&st->lock);
            st->counter += n;
            int v = st->counter;
            pthread_mutex_unlock(&st->lock);
            char resp[64];
            snprintf(resp, sizeof(resp), "OK %d", v);
            write_line(cfd, resp);
        } else if (strcmp(cmd, "DECR") == 0) {
            int n;
            if (sscanf(line, "%*s %63s", a) != 1) { write_line(cfd, "ERR ARG"); continue; }
            n = atoi(a);
            if (n < 0 || n > 1000000) { write_line(cfd, "ERR RANGE"); continue; }
            pthread_mutex_lock(&st->lock);
            if (st->counter - n < 0) {
                pthread_mutex_unlock(&st->lock);
                write_line(cfd, "ERR RANGE");
            } else {
                st->counter -= n;
                int v = st->counter;
                pthread_mutex_unlock(&st->lock);
                char resp[64];
                snprintf(resp, sizeof(resp), "OK %d", v);
                write_line(cfd, resp);
            }
        } else if (strcmp(cmd, "RESET") == 0) {
            pthread_mutex_lock(&st->lock);
            st->counter = 0;
            pthread_mutex_unlock(&st->lock);
            write_line(cfd, "OK 0");
        } else if (strcmp(cmd, "PUT") == 0) {
            // Expect: PUT key value (no spaces in value)
            if (sscanf(line, "%*s %63s %256s", a, b) != 2) { write_line(cfd, "ERR ARG"); continue; }
            if (!is_key_valid(a) || strlen(b) > MAX_VALUE) { write_line(cfd, "ERR ARG"); continue; }
            pthread_mutex_lock(&st->lock);
            kv_put(st, a, b);
            pthread_mutex_unlock(&st->lock);
            write_line(cfd, "OK");
        } else if (strcmp(cmd, "GET") == 0) {
            if (sscanf(line, "%*s %63s", a) != 1) { write_line(cfd, "ERR ARG"); continue; }
            if (!is_key_valid(a)) { write_line(cfd, "ERR ARG"); continue; }
            char val[MAX_VALUE + 1];
            bool found;
            pthread_mutex_lock(&st->lock);
            found = kv_get(st, a, val, sizeof(val));
            pthread_mutex_unlock(&st->lock);
            if (!found) write_line(cfd, "ERR NOT_FOUND");
            else {
                char resp[MAX_VALUE + 16];
                snprintf(resp, sizeof(resp), "VALUE %s", val);
                write_line(cfd, resp);
            }
        } else if (strcmp(cmd, "DEL") == 0) {
            if (sscanf(line, "%*s %63s", a) != 1) { write_line(cfd, "ERR ARG"); continue; }
            if (!is_key_valid(a)) { write_line(cfd, "ERR ARG"); continue; }
            pthread_mutex_lock(&st->lock);
            kv_del(st, a);
            pthread_mutex_unlock(&st->lock);
            write_line(cfd, "OK");
        } else if (strcmp(cmd, "LIST") == 0) {
            // Build comma-separated list
            pthread_mutex_lock(&st->lock);
            size_t buf_sz = 1;
            KVNode *cur = st->kv_head;
            while (cur) { buf_sz += strlen(cur->key) + 1; cur = cur->next; }
            char *buf = (char *)malloc(buf_sz + 16);
            if (!buf) { pthread_mutex_unlock(&st->lock); write_line(cfd, "ERR"); continue; }
            buf[0] = '\0';
            cur = st->kv_head;
            while (cur) {
                if (buf[0] != '\0') strncat(buf, ",", buf_sz - strlen(buf) - 1);
                strncat(buf, cur->key, buf_sz - strlen(buf) - 1);
                cur = cur->next;
            }
            pthread_mutex_unlock(&st->lock);
            char *resp = (char *)malloc(strlen(buf) + 6 + 2);
            if (!resp) { free(buf); write_line(cfd, "ERR"); continue; }
            strcpy(resp, "KEYS ");
            strncat(resp, buf, strlen(buf));
            write_line(cfd, resp);
            free(resp);
            free(buf);
        } else {
            write_line(cfd, "ERR CMD");
        }
    }
    close(cfd);
    return NULL;
}

ServerState *g_server_state = NULL;

static bool start_server(ServerState *st, int port, const char *secret) {
    memset(st, 0, sizeof(*st));
    st->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (st->listen_fd < 0) return false;
    int yes = 1;
    setsockopt(st->listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(st->listen_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(st->listen_fd);
        return false;
    }
    if (listen(st->listen_fd, 50) != 0) {
        close(st->listen_fd);
        return false;
    }
    pthread_mutex_init(&st->lock, NULL);
    strncpy(st->secret, secret, sizeof(st->secret) - 1);
    st->running = true;
    g_server_state = st;
    pthread_t th;
    // Accept loop thread
    pthread_create(&th, NULL, (void *(*)(void *))[](void *arg) -> void * {
        ServerState *state = (ServerState *)arg;
        while (state->running) {
            struct sockaddr_in caddr;
            socklen_t clen = sizeof(caddr);
            int *cfd = (int *)malloc(sizeof(int));
            if (!cfd) break;
            *cfd = accept(state->listen_fd, (struct sockaddr *)&caddr, &clen);
            if (*cfd < 0) {
                free(cfd);
                if (!state->running) break;
                continue;
            }
            pthread_t ct;
            pthread_create(&ct, NULL, client_thread, cfd);
            pthread_detach(ct);
        }
        return NULL;
    }, st);
    pthread_detach(th);
    return true;
}

static void stop_server(ServerState *st) {
    st->running = false;
    if (st->listen_fd >= 0) close(st->listen_fd);
    pthread_mutex_destroy(&st->lock);
    // Free KV
    KVNode *cur = st->kv_head;
    while (cur) {
        KVNode *n = cur->next;
        free(cur);
        cur = n;
    }
}

static int client_session(const char *host, int port, const char *token, const char **commands, int ncmd, char out[][512], int max_out) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return 0;
    }
    // read welcome
    char line[2048];
    int count = 0;
    if (read_line_limited(fd, line, sizeof(line), 5)) {
        snprintf(out[count++], 512, "%s", line);
    }
    // send token
    char tokline[512];
    snprintf(tokline, sizeof(tokline), "TOKEN %s", token);
    write_line(fd, tokline);
    if (read_line_limited(fd, line, sizeof(line), 5)) {
        snprintf(out[count++], 512, "%s", line);
        if (strncmp(line, "OK", 2) != 0) {
            close(fd);
            return count;
        }
    }
    for (int i = 0; i < ncmd && count < max_out; i++) {
        write_line(fd, commands[i]);
        if (read_line_limited(fd, line, sizeof(line), 5)) {
            snprintf(out[count++], 512, "%s", line);
        }
    }
    close(fd);
    return count;
}

int main() {
    const char *env = getenv("REMOTE_SECRET");
    const char *secret = env && env[0] ? env : "s3cure-Secret-Token";
    ServerState st;
    if (!start_server(&st, PORT, secret)) {
        printf("Failed to start server\n");
        return 1;
    }
    usleep(200 * 1000);

    char outputs[20][512];
    int n;

    const char *t1cmds[] = {"PING", "QUIT"};
    n = client_session("127.0.0.1", PORT, secret, t1cmds, 2, outputs, 20);
    printf("Test1:");
    for (int i = 0; i < n; i++) printf(" %s", outputs[i]);
    printf("\n");

    const char *t2cmds[] = {"GETCNT", "INCR 5", "GETCNT", "QUIT"};
    n = client_session("127.0.0.1", PORT, secret, t2cmds, 4, outputs, 20);
    printf("Test2:");
    for (int i = 0; i < n; i++) printf(" %s", outputs[i]);
    printf("\n");

    const char *t3cmds[] = {"PUT foo bar", "GET foo", "QUIT"};
    n = client_session("127.0.0.1", PORT, secret, t3cmds, 3, outputs, 20);
    printf("Test3:");
    for (int i = 0; i < n; i++) printf(" %s", outputs[i]);
    printf("\n");

    const char *t4cmds[] = {"DEL foo", "GET foo", "QUIT"};
    n = client_session("127.0.0.1", PORT, secret, t4cmds, 3, outputs, 20);
    printf("Test4:");
    for (int i = 0; i < n; i++) printf(" %s", outputs[i]);
    printf("\n");

    const char *t5cmds[] = {"PING", "QUIT"};
    n = client_session("127.0.0.1", PORT, "wrong-token", t5cmds, 2, outputs, 20);
    printf("Test5:");
    for (int i = 0; i < n; i++) printf(" %s", outputs[i]);
    printf("\n");

    stop_server(&st);
    return 0;
}