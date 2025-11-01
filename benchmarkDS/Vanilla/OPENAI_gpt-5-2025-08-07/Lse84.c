#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void rtrim(char *s) {
    int n = (int)strlen(s);
    while (n > 0 && (unsigned char)s[n - 1] <= ' ') {
        s[n - 1] = '\0';
        n--;
    }
}
static char *ltrim_inplace(char *s) {
    while (*s && (unsigned char)*s <= ' ') s++;
    return s;
}
static char *trim_inplace(char *s) {
    rtrim(s);
    return ltrim_inplace(s);
}
static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}
static void url_decode(const char *src, char *dst, size_t dstsz) {
    size_t si = 0, di = 0;
    while (src[si] && di + 1 < dstsz) {
        char c = src[si];
        if (c == '+') {
            dst[di++] = ' ';
            si++;
        } else if (c == '%' && src[si + 1] && src[si + 2]) {
            int h1 = hexval(src[si + 1]);
            int h2 = hexval(src[si + 2]);
            if (h1 >= 0 && h2 >= 0) {
                dst[di++] = (char)((h1 << 4) | h2);
                si += 3;
            } else {
                dst[di++] = c;
                si++;
            }
        } else {
            dst[di++] = c;
            si++;
        }
    }
    dst[di] = '\0';
}

static void yaml_get_type(const char *payload, char *out, size_t outsz) {
    out[0] = '\0';
    if (!payload) return;
    char *copy = strdup(payload);
    if (!copy) return;
    for (char *p = copy; *p; ++p) if (*p == '\r') *p = '\n';
    char *line = copy;
    while (line && *line) {
        char *next = strchr(line, '\n');
        if (next) *next = '\0';
        char *l = trim_inplace(line);
        if (*l && *l != '#') {
            char *colon = strchr(l, ':');
            if (colon) {
                *colon = '\0';
                char *key = trim_inplace(l);
                char *val = trim_inplace(colon + 1);
                size_t vlen = strlen(val);
                if (vlen >= 2 && ((val[0] == '"' && val[vlen - 1] == '"') || (val[0] == '\'' && val[vlen - 1] == '\''))) {
                    val[vlen - 1] = '\0';
                    val = val + 1;
                }
                if (strcmp(key, "type") == 0) {
                    strncpy(out, val, outsz - 1);
                    out[outsz - 1] = '\0';
                    break;
                }
            }
        }
        if (!next) break;
        line = next + 1;
    }
    free(copy);
}

static const char *handle_payload(const char *payload) {
    static char msg[128];
    if (payload == NULL || *trim_inplace((char *)payload) == '\0') {
        snprintf(msg, sizeof(msg), "Error: missing payload parameter");
        return msg;
    }
    char t[256];
    yaml_get_type(payload, t, sizeof(t));
    if (t[0] != '\0' && strcmp(t, "Create") == 0) {
        snprintf(msg, sizeof(msg), "Error: payload type Create is not allowed");
        return msg;
    }
    snprintf(msg, sizeof(msg), "OK");
    return msg;
}

static const char *process_query(const char *query) {
    static char msg[128];
    if (query == NULL || *query == '\0') {
        snprintf(msg, sizeof(msg), "Error: missing payload parameter");
        return msg;
    }
    // Find payload=
    const char *p = query;
    char val[4096] = {0};
    while (*p) {
        const char *amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        const char *eq = memchr(p, '=', len);
        if (eq) {
            size_t klen = (size_t)(eq - p);
            char key[256];
            size_t kcpy = klen < sizeof(key) - 1 ? klen : sizeof(key) - 1;
            memcpy(key, p, kcpy);
            key[kcpy] = '\0';
            char dkey[256];
            url_decode(key, dkey, sizeof(dkey));
            if (strcmp(dkey, "payload") == 0) {
                size_t vlen = len - klen - 1;
                char enc[4096];
                size_t vcpy = vlen < sizeof(enc) - 1 ? vlen : sizeof(enc) - 1;
                memcpy(enc, eq + 1, vcpy);
                enc[vcpy] = '\0';
                url_decode(enc, val, sizeof(val));
                break;
            }
        }
        if (!amp) break;
        p = amp + 1;
    }
    if (val[0] == '\0') {
        snprintf(msg, sizeof(msg), "Error: missing payload parameter");
        return msg;
    }
    return handle_payload(val);
}

struct client_arg { int fd; };

static void *client_thread(void *arg) {
    int cfd = ((struct client_arg *)arg)->fd;
    free(arg);
    char buf[4096];
    ssize_t n = read(cfd, buf, sizeof(buf) - 1);
    if (n <= 0) {
        close(cfd);
        return NULL;
    }
    buf[n] = '\0';
    char *first_line_end = strstr(buf, "\r\n");
    if (!first_line_end) first_line_end = strchr(buf, '\n');
    char reqline[1024] = {0};
    if (first_line_end) {
        size_t len = (size_t)(first_line_end - buf);
        size_t cpy = len < sizeof(reqline) - 1 ? len : sizeof(reqline) - 1;
        memcpy(reqline, buf, cpy);
        reqline[cpy] = '\0';
    } else {
        strncpy(reqline, buf, sizeof(reqline) - 1);
    }
    // Parse GET path
    char method[16] = {0}, path[1024] = {0}, proto[16] = {0};
    sscanf(reqline, "%15s %1023s %15s", method, path, proto);
    char *query = NULL;
    char *qmark = strchr(path, '?');
    if (qmark) query = qmark + 1;
    const char *body = process_query(query);
    char resp[5120];
    int blen = (int)strlen(body);
    int rlen = snprintf(resp, sizeof(resp),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain; charset=utf-8\r\n"
                        "Content-Length: %d\r\n"
                        "Connection: close\r\n\r\n%s",
                        blen, body);
    if (rlen > 0) {
        send(cfd, resp, (size_t)rlen, 0);
    }
    close(cfd);
    return NULL;
}

static void *server_loop(void *unused) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return NULL;
    }
    if (listen(fd, 16) != 0) {
        close(fd);
        return NULL;
    }
    while (1) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd < 0) continue;
        pthread_t th;
        struct client_arg *ca = (struct client_arg *)malloc(sizeof(struct client_arg));
        if (!ca) { close(cfd); continue; }
        ca->fd = cfd;
        pthread_create(&th, NULL, client_thread, ca);
        pthread_detach(th);
    }
    // close(fd);
    // return NULL;
}

int main() {
    pthread_t th;
    pthread_create(&th, NULL, server_loop, NULL);
    pthread_detach(th);

    // 5 test cases
    printf("%s\n", handle_payload(""));                 // Missing -> error
    printf("%s\n", handle_payload("type: Create"));     // Create -> error
    printf("%s\n", handle_payload("type: Update"));     // OK
    printf("%s\n", handle_payload("name: x"));          // OK
    printf("%s\n", handle_payload("TYPE: Create"));     // OK

    while (1) pause();
    return 0;
}