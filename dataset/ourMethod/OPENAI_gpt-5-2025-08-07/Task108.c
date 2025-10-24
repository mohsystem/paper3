#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_OBJECTS 64
#define MAX_FIELDS 64
#define ID_KEY_MAX 64
#define VAL_MAX 256

typedef struct {
    bool used;
    char key[ID_KEY_MAX + 1];
    char value[VAL_MAX + 1];
} Field;

typedef struct {
    bool used;
    char id[ID_KEY_MAX + 1];
    Field fields[MAX_FIELDS];
} Object;

static Object g_store[MAX_OBJECTS];
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile int g_listen_fd = -1;

static bool is_valid_id_or_key(const char *s) {
    if (!s) return false;
    size_t n = strlen(s);
    if (n < 1 || n > ID_KEY_MAX) return false;
    for (size_t i = 0; i < n; ++i) {
        char c = s[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') || c == '_')) return false;
    }
    return true;
}

static bool is_valid_value(const char *s) {
    if (!s) return false;
    size_t n = strlen(s);
    if (n > VAL_MAX) return false;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c == ' ') continue;
        if (c < 33 || c > 126) return false;
    }
    return true;
}

static Object* find_object(const char *id) {
    for (int i = 0; i < MAX_OBJECTS; ++i) {
        if (g_store[i].used && strncmp(g_store[i].id, id, ID_KEY_MAX) == 0) {
            return &g_store[i];
        }
    }
    return NULL;
}

static Object* create_object(const char *id) {
    for (int i = 0; i < MAX_OBJECTS; ++i) {
        if (!g_store[i].used) {
            g_store[i].used = true;
            strncpy(g_store[i].id, id, ID_KEY_MAX);
            g_store[i].id[ID_KEY_MAX] = '\0';
            for (int j = 0; j < MAX_FIELDS; ++j) g_store[i].fields[j].used = false;
            return &g_store[i];
        }
    }
    return NULL;
}

static Field* find_field(Object *obj, const char *key) {
    for (int i = 0; i < MAX_FIELDS; ++i) {
        if (obj->fields[i].used && strncmp(obj->fields[i].key, key, ID_KEY_MAX) == 0) {
            return &obj->fields[i];
        }
    }
    return NULL;
}

static Field* create_field(Object *obj, const char *key) {
    for (int i = 0; i < MAX_FIELDS; ++i) {
        if (!obj->fields[i].used) {
            obj->fields[i].used = true;
            strncpy(obj->fields[i].key, key, ID_KEY_MAX);
            obj->fields[i].key[ID_KEY_MAX] = '\0';
            obj->fields[i].value[0] = '\0';
            return &obj->fields[i];
        }
    }
    return NULL;
}

static void respondf(int fd, const char *fmt, ...) {
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    size_t len = strnlen(buf, sizeof(buf));
    send(fd, buf, len, 0);
    send(fd, "\n", 1, 0);
}

static bool read_line_limited(int fd, char *out, size_t maxlen) {
    size_t n = 0;
    while (n < maxlen) {
        char c;
        ssize_t r = recv(fd, &c, 1, 0);
        if (r == 0) return false; // closed
        if (r < 0) return false;
        if (c == '\n') break;
        if (c == '\r') continue;
        out[n++] = c;
    }
    if (n >= maxlen) return false;
    out[n] = '\0';
    return true;
}

static void trim(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t')) {
        s[len - 1] = '\0';
        len--;
    }
}

static int split_spaces(const char *line, char parts[][VAL_MAX + 1], int max_parts) {
    int count = 0;
    size_t i = 0;
    size_t n = strlen(line);
    while (i < n && count < max_parts) {
        while (i < n && line[i] == ' ') i++;
        if (i >= n) break;
        size_t start = i;
        while (i < n && line[i] != ' ') i++;
        size_t len = i - start;
        if (len > VAL_MAX) len = VAL_MAX;
        strncpy(parts[count], line + start, len);
        parts[count][len] = '\0';
        count++;
    }
    return count;
}

static void process_command(const char *line, int fd) {
    char parts[4][VAL_MAX + 1];
    int n = split_spaces(line, parts, 4);
    if (n <= 0) {
        respondf(fd, "ERR empty");
        return;
    }
    for (int i = 0; parts[0][i]; ++i) {
        if (parts[0][i] >= 'a' && parts[0][i] <= 'z') parts[0][i] = (char)(parts[0][i] - 32);
    }
    if (strcmp(parts[0], "PING") == 0) {
        respondf(fd, "OK PONG");
        return;
    } else if (strcmp(parts[0], "NEW") == 0 && n == 2) {
        if (!is_valid_id_or_key(parts[1])) { respondf(fd, "ERR invalid_id"); return; }
        pthread_mutex_lock(&g_lock);
        Object *o = find_object(parts[1]);
        if (o) {
            pthread_mutex_unlock(&g_lock);
            respondf(fd, "ERR exists");
            return;
        }
        o = create_object(parts[1]);
        pthread_mutex_unlock(&g_lock);
        respondf(fd, o ? "OK" : "ERR full");
        return;
    } else if (strcmp(parts[0], "PUT") == 0 && n == 4) {
        if (!is_valid_id_or_key(parts[1])) { respondf(fd, "ERR invalid_id"); return; }
        if (!is_valid_id_or_key(parts[2])) { respondf(fd, "ERR invalid_key"); return; }
        if (!is_valid_value(parts[3])) { respondf(fd, "ERR invalid_value"); return; }
        pthread_mutex_lock(&g_lock);
        Object *o = find_object(parts[1]);
        if (!o) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        Field *f = find_field(o, parts[2]);
        if (!f) f = create_field(o, parts[2]);
        if (!f) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR full"); return; }
        strncpy(f->value, parts[3], VAL_MAX);
        f->value[VAL_MAX] = '\0';
        pthread_mutex_unlock(&g_lock);
        respondf(fd, "OK");
        return;
    } else if (strcmp(parts[0], "GET") == 0 && n == 3) {
        if (!is_valid_id_or_key(parts[1]) || !is_valid_id_or_key(parts[2])) { respondf(fd, "ERR invalid"); return; }
        pthread_mutex_lock(&g_lock);
        Object *o = find_object(parts[1]);
        if (!o) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        Field *f = find_field(o, parts[2]);
        if (!f) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        char resp[VAL_MAX + 5];
        snprintf(resp, sizeof(resp), "OK %s", f->value);
        pthread_mutex_unlock(&g_lock);
        respondf(fd, "%s", resp);
        return;
    } else if (strcmp(parts[0], "DEL") == 0 && n == 3) {
        if (!is_valid_id_or_key(parts[1]) || !is_valid_id_or_key(parts[2])) { respondf(fd, "ERR invalid"); return; }
        pthread_mutex_lock(&g_lock);
        Object *o = find_object(parts[1]);
        if (!o) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        Field *f = find_field(o, parts[2]);
        if (!f) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        f->used = false;
        pthread_mutex_unlock(&g_lock);
        respondf(fd, "OK");
        return;
    } else if (strcmp(parts[0], "KEYS") == 0 && n == 2) {
        if (!is_valid_id_or_key(parts[1])) { respondf(fd, "ERR invalid_id"); return; }
        pthread_mutex_lock(&g_lock);
        Object *o = find_object(parts[1]);
        if (!o) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        char buf[1024];
        buf[0] = '\0';
        size_t pos = 0;
        for (int i = 0; i < MAX_FIELDS; ++i) {
            if (o->fields[i].used) {
                size_t klen = strnlen(o->fields[i].key, ID_KEY_MAX);
                if (pos + klen + 2 >= sizeof(buf)) break;
                if (pos != 0) { buf[pos++] = ','; }
                memcpy(&buf[pos], o->fields[i].key, klen);
                pos += klen;
                buf[pos] = '\0';
            }
        }
        pthread_mutex_unlock(&g_lock);
        respondf(fd, "OK %s", buf);
        return;
    } else if (strcmp(parts[0], "DROP") == 0 && n == 2) {
        if (!is_valid_id_or_key(parts[1])) { respondf(fd, "ERR invalid_id"); return; }
        pthread_mutex_lock(&g_lock);
        Object *o = find_object(parts[1]);
        if (!o) { pthread_mutex_unlock(&g_lock); respondf(fd, "ERR not_found"); return; }
        o->used = false;
        pthread_mutex_unlock(&g_lock);
        respondf(fd, "OK");
        return;
    } else if (strcmp(parts[0], "QUIT") == 0) {
        respondf(fd, "OK BYE");
        return;
    }
    respondf(fd, "ERR unknown");
}

typedef struct {
    int fd;
} client_arg_t;

static void* client_thread(void *arg) {
    client_arg_t *c = (client_arg_t*)arg;
    int fd = c->fd;
    free(c);
    char line[MAX_LINE + 1];
    while (1) {
        if (!read_line_limited(fd, line, MAX_LINE)) break;
        trim(line);
        if (line[0] == '\0') { respondf(fd, "ERR empty"); continue; }
        process_command(line, fd);
        if (strncmp(line, "QUIT", 4) == 0) break;
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);
    return NULL;
}

static uint16_t start_server(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return 0;
    }
    if (listen(fd, 16) < 0) {
        close(fd);
        return 0;
    }
    socklen_t len = sizeof(addr);
    if (getsockname(fd, (struct sockaddr*)&addr, &len) == 0) {
        // ok
    }
    g_listen_fd = fd;
    uint16_t actual_port = ntohs(addr.sin_port);
    pthread_t thr;
    pthread_create(&thr, NULL, (void*(*)(void*)) (void*) ({
        void* _accept_loop(void* p) {
            (void)p;
            while (1) {
                int cfd = accept(g_listen_fd, NULL, NULL);
                if (cfd < 0) break;
                client_arg_t *arg = (client_arg_t*)malloc(sizeof(client_arg_t));
                if (!arg) { close(cfd); continue; }
                arg->fd = cfd;
                pthread_t t;
                pthread_create(&t, NULL, client_thread, arg);
                pthread_detach(t);
            }
            return (void*)0;
        }
        _accept_loop;
    }), NULL);
    pthread_detach(thr);
    return actual_port;
}

static void stop_server(void) {
    if (g_listen_fd >= 0) {
        shutdown(g_listen_fd, SHUT_RDWR);
        close(g_listen_fd);
        g_listen_fd = -1;
    }
}

static int send_commands(const char *host, uint16_t port, const char **cmds, int ncmds, char resp[][MAX_LINE + 1]) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, host, &addr.sin_addr);
    addr.sin_port = htons(port);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    for (int i = 0; i < ncmds; ++i) {
        char line[MAX_LINE + 1];
        snprintf(line, sizeof(line), "%s\n", cmds[i]);
        send(fd, line, strnlen(line, sizeof(line)), 0);
        if (!read_line_limited(fd, resp[i], MAX_LINE)) {
            snprintf(resp[i], MAX_LINE, "%s", "ERR no_response");
        }
    }
    const char *q = "QUIT\n";
    send(fd, q, strlen(q), 0);
    char tmp[MAX_LINE + 1];
    read_line_limited(fd, tmp, MAX_LINE);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    return 0;
}

int main(void) {
    uint16_t port = start_server(0);
    if (port == 0) {
        printf("Server error\n");
        return 0;
    }

    // Test1
    const char *c1[] = {"NEW objA"};
    char r1[1][MAX_LINE + 1];
    send_commands("127.0.0.1", port, c1, 1, r1);
    printf("Test1: %s\n", r1[0]);

    // Test2
    const char *c2[] = {"PUT objA key1 v1"};
    char r2[1][MAX_LINE + 1];
    send_commands("127.0.0.1", port, c2, 1, r2);
    printf("Test2: %s\n", r2[0]);

    // Test3
    const char *c3[] = {"GET objA key1"};
    char r3[1][MAX_LINE + 1];
    send_commands("127.0.0.1", port, c3, 1, r3);
    printf("Test3: %s\n", r3[0]);

    // Test4
    const char *c4[] = {"KEYS objA"};
    char r4[1][MAX_LINE + 1];
    send_commands("127.0.0.1", port, c4, 1, r4);
    printf("Test4: %s\n", r4[0]);

    // Test5
    const char *c5[] = {"DEL objA key1", "GET objA key1"};
    char r5[2][MAX_LINE + 1];
    send_commands("127.0.0.1", port, c5, 2, r5);
    printf("Test5a: %s\n", r5[0]);
    printf("Test5b: %s\n", r5[1]);

    stop_server();
    return 0;
}