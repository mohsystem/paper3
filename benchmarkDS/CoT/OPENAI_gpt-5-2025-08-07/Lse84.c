/*
Chain-of-Through in code (comments only):
1) Problem: HTTP server on port 5000; check 'payload' parameter; parse YAML; if type == Create -> error.
2) Security: Input caps, minimal YAML subset, safe decoding, handle only GET.
3) Implementation: POSIX sockets, simple parser and response builder.
4) Review: Defensive checks, length limits, avoid dangerous YAML constructs.
5) Final: Below code with 5 core logic tests (printed) and server loop.
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *strdup_s(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

static char *trim_copy(const char *s) {
    if (!s) return strdup_s("");
    const char *b = s;
    const char *e = s + strlen(s);
    while (b < e && isspace((unsigned char)*b)) b++;
    while (e > b && isspace((unsigned char)*(e - 1))) e--;
    size_t n = (size_t)(e - b);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, b, n);
    out[n] = '\0';
    return out;
}

static char hexval(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'a' && c <= 'f') return (char)(10 + c - 'a');
    if (c >= 'A' && c <= 'F') return (char)(10 + c - 'A');
    return (char)-1;
}

static char *url_decode(const char *s) {
    size_t n = strlen(s);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = s[i];
        if (c == '%' && i + 2 < n) {
            char h1 = s[i + 1], h2 = s[i + 2];
            char hi = hexval(h1), lo = hexval(h2);
            if (hi >= 0 && lo >= 0) {
                out[oi++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                out[oi++] = c;
            }
        } else if (c == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = (char)c;
        }
    }
    out[oi] = '\0';
    return out;
}

typedef struct {
    char **keys;
    char **vals;
    size_t len;
    size_t cap;
} map_t;

static void map_init(map_t *m) {
    m->keys = NULL; m->vals = NULL; m->len = 0; m->cap = 0;
}
static void map_free(map_t *m) {
    if (!m) return;
    for (size_t i = 0; i < m->len; i++) {
        free(m->keys[i]);
        free(m->vals[i]);
    }
    free(m->keys);
    free(m->vals);
}
static bool map_put(map_t *m, const char *k, const char *v) {
    if (m->len == m->cap) {
        size_t nc = m->cap ? m->cap * 2 : 8;
        char **nk = (char **)realloc(m->keys, nc * sizeof(char *));
        char **nv = (char **)realloc(m->vals, nc * sizeof(char *));
        if (!nk || !nv) return false;
        m->keys = nk; m->vals = nv; m->cap = nc;
    }
    m->keys[m->len] = strdup_s(k);
    m->vals[m->len] = strdup_s(v);
    if (!m->keys[m->len] || !m->vals[m->len]) return false;
    m->len++;
    return true;
}
static const char *map_get(map_t *m, const char *k) {
    for (size_t i = 0; i < m->len; i++) {
        if (strcmp(m->keys[i], k) == 0) return m->vals[i];
    }
    return NULL;
}

static char *unquote(const char *v) {
    size_t n = strlen(v);
    if (n >= 2 && ((v[0] == '"' && v[n-1] == '"') || (v[0] == '\'' && v[n-1] == '\''))) {
        char *o = (char *)malloc(n - 1);
        if (!o) return NULL;
        memcpy(o, v + 1, n - 2);
        o[n - 2] = '\0';
        return o;
    }
    return strdup_s(v);
}

static bool parse_simple_yaml_map(const char *yaml, map_t *out) {
    map_init(out);
    if (!yaml) return false;
    size_t len = strlen(yaml);
    if (len > 10000) return false;
    char *copy = strdup_s(yaml);
    if (!copy) return false;
    size_t lines = 0, total = 0;
    char *save = NULL;
    char *line = strtok_r(copy, "\n", &save);
    bool ok = true;
    while (line) {
        if (++lines > 1000) { ok = false; break; }
        total += strlen(line);
        if (total > 10000) { ok = false; break; }
        char *t = trim_copy(line);
        if (!t) { ok = false; break; }
        if (t[0] == '\0' || t[0] == '#') {
            free(t);
            line = strtok_r(NULL, "\n", &save);
            continue;
        }
        char *colon = strchr(t, ':');
        if (!colon || colon == t) { free(t); ok = false; break; }
        *colon = '\0';
        char *key = trim_copy(t);
        char *val_raw = trim_copy(colon + 1);
        free(t);
        if (!key || !val_raw || key[0] == '\0') { free(key); free(val_raw); ok = false; break; }
        char *val = unquote(val_raw);
        free(val_raw);
        if (!val) { free(key); ok = false; break; }
        if (val[0] == '&' || val[0] == '*' || val[0] == '!') { free(key); free(val); ok = false; break; }
        if (!map_put(out, key, val)) { free(key); free(val); ok = false; break; }
        free(key); free(val);
        line = strtok_r(NULL, "\n", &save);
    }
    free(copy);
    if (!ok) {
        map_free(out);
        return false;
    }
    return true;
}

static char *process_payload(const char *payload) {
    if (!payload || payload[0] == '\0') {
        return strdup_s("Error: Missing payload parameter");
    }
    if (strlen(payload) > 10000) {
        return strdup_s("Error: Payload too large");
    }
    map_t mp;
    if (!parse_simple_yaml_map(payload, &mp)) {
        return strdup_s("Error: Invalid YAML");
    }
    const char *type = map_get(&mp, "type");
    char *res = NULL;
    if (type && strcmp(type, "Create") == 0) {
        res = strdup_s("Error: Create type is not allowed");
    } else {
        res = strdup_s("OK");
    }
    map_free(&mp);
    return res;
}

static void send_http_response(int client_fd, int status, const char *body) {
    char header[256];
    size_t body_len = strlen(body);
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     status, status == 200 ? "OK" : "Error", body_len);
    send(client_fd, header, (size_t)n, 0);
    send(client_fd, body, body_len, 0);
}

static void handle_client(int client_fd) {
    char buf[8192];
    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) { close(client_fd); return; }
    buf[n] = '\0';
    // Parse request line
    char *line_end = strstr(buf, "\r\n");
    if (!line_end) { send_http_response(client_fd, 400, "Error: Bad Request"); close(client_fd); return; }
    *line_end = '\0';
    char method[8], path[4096], version[16];
    method[0]=path[0]=version[0]='\0';
    sscanf(buf, "%7s %4095s %15s", method, path, version);
    if (strcmp(method, "GET") != 0) {
        send_http_response(client_fd, 405, "Error: Method Not Allowed");
        close(client_fd);
        return;
    }
    // Extract query
    char *qmark = strchr(path, '?');
    char *payload_decoded = NULL;
    if (qmark) {
        char *query = qmark + 1;
        // Find payload=
        char *q = query;
        while (*q) {
            char *amp = strchr(q, '&');
            size_t part_len = amp ? (size_t)(amp - q) : strlen(q);
            char *part = (char *)malloc(part_len + 1);
            if (!part) break;
            memcpy(part, q, part_len);
            part[part_len] = '\0';
            char *eq = strchr(part, '=');
            if (eq) {
                *eq = '\0';
                char *name = part;
                char *val = eq + 1;
                char *name_dec = url_decode(name);
                if (name_dec && strcmp(name_dec, "payload") == 0 && !payload_decoded) {
                    char *val_dec = url_decode(val);
                    if (val_dec) {
                        if (strlen(val_dec) > 10000) val_dec[10000] = '\0';
                        payload_decoded = val_dec;
                    }
                }
                free(name_dec);
            }
            free(part);
            if (!amp) break;
            q = amp + 1;
        }
    }
    if (!payload_decoded) {
        send_http_response(client_fd, 400, "Error: Missing payload parameter");
        close(client_fd);
        return;
    }
    char *result = process_payload(payload_decoded);
    int status = (strncmp(result, "Error:", 6) == 0) ? 400 : 200;
    send_http_response(client_fd, status, result);
    free(payload_decoded);
    free(result);
    close(client_fd);
}

static void *server_thread(void *arg) {
    (void)arg;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return NULL;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return NULL;
    }
    if (listen(server_fd, 16) < 0) {
        perror("listen");
        close(server_fd);
        return NULL;
    }
    printf("C server listening on http://0.0.0.0:5000/\n");
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) continue;
        handle_client(client_fd);
    }
    close(server_fd);
    return NULL;
}

int main(void) {
    pthread_t tid;
    pthread_create(&tid, NULL, server_thread, NULL);
    pthread_detach(tid);

    // 5 test cases for core logic
    const char *tests[5] = {
        "type: Create",
        "type: Update",
        "foo: bar",
        "type: 'Create'",
        "type Create"
    };
    for (int i = 0; i < 5; i++) {
        char *out = process_payload(tests[i]);
        printf("Test %d => %s\n", i + 1, out);
        free(out);
    }

    // Keep main alive
    while (1) {
        sleep(60);
    }
    return 0;
}