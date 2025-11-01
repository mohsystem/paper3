#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 5000
#define MAX_INPUT_SIZE 65536

static volatile int g_listen_fd = -1;
static volatile int g_stop = 0;

static void trim(char *s) {
    size_t len = strlen(s);
    size_t b = 0;
    while (b < len && (s[b] == ' ' || s[b] == '\t')) b++;
    size_t e = len;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) e--;
    if (b > 0) memmove(s, s + b, e - b);
    s[e - b] = '\0';
}

static int is_valid_key(const char *k) {
    size_t n = strlen(k);
    if (n == 0 || n > 64) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)k[i];
        if (!(isalnum(c) || c == '_')) return 0;
    }
    return 1;
}

typedef struct {
    char keys[64][65];
    char vals[64][4097];
    int count;
} yaml_map_t;

static int parse_simple_yaml(const char *src, yaml_map_t *out) {
    if (!src || !out) return 0;
    size_t len = strlen(src);
    if (len > MAX_INPUT_SIZE) return 0;
    out->count = 0;
    char *copy = (char *)malloc(len + 1);
    if (!copy) return 0;
    memcpy(copy, src, len + 1);
    char *line = strtok(copy, "\n");
    while (line) {
        // remove trailing \r
        size_t ll = strlen(line);
        if (ll > 0 && line[ll - 1] == '\r') line[ll - 1] = '\0';
        char buf[8192];
        snprintf(buf, sizeof(buf), "%s", line);
        trim(buf);
        if (buf[0] != '\0' && buf[0] != '#') {
            char *colon = strchr(buf, ':');
            if (!colon || colon == buf) {
                free(copy);
                return 0;
            }
            *colon = '\0';
            char *key = buf;
            char *val = colon + 1;
            trim(key);
            trim(val);
            if (!is_valid_key(key)) {
                free(copy);
                return 0;
            }
            // remove simple quotes
            size_t vl = strlen(val);
            if (vl >= 2 && ((val[0] == '"' && val[vl - 1] == '"') || (val[0] == '\'' && val[vl - 1] == '\''))) {
                val[vl - 1] = '\0';
                val++;
            }
            if (strlen(val) > 4096) {
                free(copy);
                return 0;
            }
            if (out->count >= 64) {
                free(copy);
                return 0;
            }
            snprintf(out->keys[out->count], sizeof(out->keys[out->count]), "%s", key);
            snprintf(out->vals[out->count], sizeof(out->vals[out->count]), "%s", val);
            out->count++;
        }
        line = strtok(NULL, "\n");
    }
    free(copy);
    return out->count > 0;
}

static char hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'a' && c <= 'f') return (char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (char)(c - 'A' + 10);
    return -1;
}

static void url_decode(const char *in, char *out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0' && oi + 1 < outsz; i++) {
        char c = in[i];
        if (c == '+') {
            out[oi++] = ' ';
        } else if (c == '%' && in[i + 1] && in[i + 2]) {
            char a = hex_to_nibble(in[i + 1]);
            char b = hex_to_nibble(in[i + 2]);
            if (a >= 0 && b >= 0) {
                out[oi++] = (char)((a << 4) | b);
                i += 2;
            } else {
                out[oi++] = c;
            }
        } else {
            out[oi++] = c;
        }
    }
    out[oi] = '\0';
}

typedef struct {
    char keys[64][101];
    char vals[64][MAX_INPUT_SIZE + 1];
    int count;
} kv_map_t;

static void parse_query(const char *q, kv_map_t *out) {
    out->count = 0;
    if (!q || !*q) return;
    const char *p = q;
    while (*p && out->count < 64) {
        const char *amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        char pair[8192];
        if (len >= sizeof(pair)) len = sizeof(pair) - 1;
        memcpy(pair, p, len);
        pair[len] = '\0';
        const char *eq = strchr(pair, '=');
        char kraw[1024] = {0}, vraw[MAX_INPUT_SIZE + 1] = {0};
        if (eq) {
            size_t klen = (size_t)(eq - pair);
            if (klen >= sizeof(kraw)) klen = sizeof(kraw) - 1;
            memcpy(kraw, pair, klen);
            kraw[klen] = '\0';
            snprintf(vraw, sizeof(vraw), "%s", eq + 1);
        } else {
            snprintf(kraw, sizeof(kraw), "%s", pair);
            vraw[0] = '\0';
        }
        char key[101] = {0}, val[MAX_INPUT_SIZE + 1] = {0};
        url_decode(kraw, key, sizeof(key));
        url_decode(vraw, val, sizeof(val));
        if (key[0] != '\0') {
            snprintf(out->keys[out->count], sizeof(out->keys[out->count]), "%s", key);
            snprintf(out->vals[out->count], sizeof(out->vals[out->count]), "%s", val);
            out->count++;
        }
        if (!amp) break;
        p = amp + 1;
    }
}

static void process_request(const kv_map_t *params, int *status, char *out, size_t outsz) {
    const char *payload = NULL;
    for (int i = 0; i < params->count; i++) {
        if (strcmp(params->keys[i], "payload") == 0) {
            payload = params->vals[i];
            break;
        }
    }
    if (!payload) {
        *status = 400;
        snprintf(out, outsz, "Error: missing payload parameter");
        return;
    }
    if (strlen(payload) > MAX_INPUT_SIZE) {
        *status = 413;
        snprintf(out, outsz, "Error: payload too large");
        return;
    }
    yaml_map_t ymap;
    if (!parse_simple_yaml(payload, &ymap)) {
        *status = 400;
        snprintf(out, outsz, "Error: invalid payload format");
        return;
    }
    for (int i = 0; i < ymap.count; i++) {
        if (strcmp(ymap.keys[i], "type") == 0 && strcmp(ymap.vals[i], "Create") == 0) {
            *status = 400;
            snprintf(out, outsz, "Error: operation not allowed");
            return;
        }
    }
    *status = 200;
    snprintf(out, outsz, "OK");
}

static void send_http_response(int cfd, int code, const char *msg) {
    const char *status = (code == 200) ? "OK" : (code == 400) ? "Bad Request" : (code == 413) ? "Payload Too Large" : "Error";
    char body[4096];
    snprintf(body, sizeof(body), "%s\n", msg);
    char hdr[1024];
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 %d %s\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     code, status, strlen(body));
    send(cfd, hdr, n, 0);
    send(cfd, body, (int)strlen(body), 0);
}

static void handle_client(int cfd) {
    char buf[8192];
    char req[MAX_INPUT_SIZE * 2 + 1];
    size_t used = 0;
    while (1) {
        ssize_t r = recv(cfd, buf, sizeof(buf), 0);
        if (r <= 0) break;
        if (used + (size_t)r >= sizeof(req)) break;
        memcpy(req + used, buf, (size_t)r);
        used += (size_t)r;
        req[used] = '\0';
        if (strstr(req, "\r\n\r\n")) break;
    }
    char *hdr_end = strstr(req, "\r\n\r\n");
    if (!hdr_end) {
        send_http_response(cfd, 400, "Error: invalid request");
        close(cfd);
        return;
    }
    size_t header_len = (size_t)(hdr_end - req);
    char header[8192];
    size_t hl = header_len < sizeof(header) - 1 ? header_len : sizeof(header) - 1;
    memcpy(header, req, hl);
    header[hl] = '\0';

    char method[8] = {0}, path[2048] = {0}, proto[16] = {0};
    {
        char *saveptr = NULL;
        char *line = strtok_r(header, "\r\n", &saveptr);
        if (!line) {
            send_http_response(cfd, 400, "Error: invalid request");
            close(cfd);
            return;
        }
        sscanf(line, "%7s %2047s %15s", method, path, proto);
    }

    size_t content_length = 0;
    char content_type[256] = {0};
    {
        char *saveptr = NULL;
        char *line = strtok_r(NULL, "", &saveptr); // remainder headers block
        if (line) {
            // iterate lines
            char *p = line;
            while (p && *p) {
                char *nl = strstr(p, "\r\n");
                if (nl) *nl = '\0';
                char key[256] = {0}, val[768] = {0};
                const char *colon = strchr(p, ':');
                if (colon) {
                    size_t klen = (size_t)(colon - p);
                    if (klen >= sizeof(key)) klen = sizeof(key) - 1;
                    memcpy(key, p, klen);
                    key[klen] = '\0';
                    snprintf(val, sizeof(val), "%s", colon + 1);
                    trim(val);
                    for (char *q = key; *q; ++q) *q = (char)tolower((unsigned char)*q);
                    if (strcmp(key, "content-length") == 0) {
                        content_length = (size_t)strtoul(val, NULL, 10);
                    } else if (strcmp(key, "content-type") == 0) {
                        snprintf(content_type, sizeof(content_type), "%s", val);
                    }
                }
                if (!nl) break;
                p = nl + 2;
            }
        }
    }

    kv_map_t params;
    params.count = 0;
    // Query from path
    char *qm = strchr(path, '?');
    if (qm) {
        *qm = '\0';
        parse_query(qm + 1, &params);
    }

    // Read body if any
    char bodybuf[MAX_INPUT_SIZE + 1];
    size_t have = used - (header_len + 4);
    size_t to_copy = have < content_length ? have : content_length;
    if (to_copy > MAX_INPUT_SIZE) to_copy = MAX_INPUT_SIZE;
    memcpy(bodybuf, hdr_end + 4, to_copy);
    size_t total = to_copy;
    while (total < content_length && total < MAX_INPUT_SIZE) {
        ssize_t r = recv(cfd, buf, sizeof(buf), 0);
        if (r <= 0) break;
        size_t chunk = (size_t)r;
        if (total + chunk > MAX_INPUT_SIZE) chunk = MAX_INPUT_SIZE - total;
        memcpy(bodybuf + total, buf, chunk);
        total += chunk;
    }
    bodybuf[total] = '\0';

    if (content_length > MAX_INPUT_SIZE) {
        send_http_response(cfd, 413, "Error: payload too large");
        close(cfd);
        return;
    }

    // Content-Type handling
    if (total > 0) {
        char ctype_l[256];
        snprintf(ctype_l, sizeof(ctype_l), "%s", content_type);
        for (char *q = ctype_l; *q; ++q) *q = (char)tolower((unsigned char)*q);
        if (strncmp(ctype_l, "application/x-www-form-urlencoded", 33) == 0) {
            kv_map_t body_params;
            parse_query(bodybuf, &body_params);
            for (int i = 0; i < body_params.count && params.count < 64; i++) {
                snprintf(params.keys[params.count], sizeof(params.keys[params.count]), "%s", body_params.keys[i]);
                snprintf(params.vals[params.count], sizeof(params.vals[params.count]), "%s", body_params.vals[i]);
                params.count++;
            }
        } else if (strncmp(ctype_l, "text/plain", 10) == 0) {
            if (params.count < 64) {
                snprintf(params.keys[params.count], sizeof(params.keys[params.count]), "%s", "payload");
                snprintf(params.vals[params.count], sizeof(params.vals[params.count]), "%s", bodybuf);
                params.count++;
            }
        }
    }

    int status = 500;
    char msg[256];
    process_request(&params, &status, msg, sizeof(msg));
    send_http_response(cfd, status, msg);
    close(cfd);
}

static void *server_thread(void *arg) {
    (void)arg;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return NULL;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sfd); return NULL;
    }
    if (listen(sfd, 16) < 0) {
        close(sfd); return NULL;
    }
    g_listen_fd = sfd;
    while (!g_stop) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sfd, &rfds);
        struct timeval tv;
        tv.tv_sec = 0; tv.tv_usec = 300000;
        int r = select(sfd + 1, &rfds, NULL, NULL, &tv);
        if (r > 0 && FD_ISSET(sfd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t cl = sizeof(cli);
            int cfd = accept(sfd, (struct sockaddr *)&cli, &cl);
            if (cfd >= 0) {
                handle_client(cfd);
            }
        }
    }
    close(sfd);
    g_listen_fd = -1;
    return NULL;
}

static void url_encode(const char *in, char *out, size_t outsz) {
    static const char *hex = "0123456789ABCDEF";
    size_t oi = 0;
    for (size_t i = 0; in[i] && oi + 4 < outsz; i++) {
        unsigned char c = (unsigned char)in[i];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
            out[oi++] = (char)c;
        } else if (c == ' ') {
            out[oi++] = '+';
        } else {
            out[oi++] = '%';
            out[oi++] = hex[c >> 4];
            out[oi++] = hex[c & 0xF];
        }
    }
    out[oi] = '\0';
}

static void http_request_test(const char *method, const char *path, const char *body, const char *ctype) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("socket error\n");
        return;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("connect error\n");
        close(fd);
        return;
    }
    char reqhdr[4096];
    int n = snprintf(reqhdr, sizeof(reqhdr),
                     "%s %s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n",
                     method, path);
    send(fd, reqhdr, n, 0);
    if (body && *body) {
        char hdr[512];
        int hn = snprintf(hdr, sizeof(hdr), "Content-Length: %zu\r\n", strlen(body));
        send(fd, hdr, hn, 0);
        if (ctype && *ctype) {
            int cn = snprintf(hdr, sizeof(hdr), "Content-Type: %s\r\n", ctype);
            send(fd, hdr, cn, 0);
        }
    }
    send(fd, "\r\n", 2, 0);
    if (body && *body) {
        send(fd, body, (int)strlen(body), 0);
    }
    char buf[4096];
    char resp[16384];
    size_t used = 0;
    ssize_t r;
    while ((r = recv(fd, buf, sizeof(buf), 0)) > 0) {
        if (used + (size_t)r >= sizeof(resp)) break;
        memcpy(resp + used, buf, (size_t)r);
        used += (size_t)r;
    }
    resp[used] = '\0';
    close(fd);
    int code = -1;
    char *sp = strchr(resp, ' ');
    if (sp && sp + 4 <= resp + used) code = atoi(sp + 1);
    char *hdr_end = strstr(resp, "\r\n\r\n");
    const char *bodyp = hdr_end ? hdr_end + 4 : "";
    printf("[%s %s] -> HTTP %d | Body: %.120s\n", method, path, code, bodyp);
}

int main(void) {
    pthread_t tid;
    if (pthread_create(&tid, NULL, server_thread, NULL) != 0) {
        fprintf(stderr, "failed to start server\n");
        return 1;
    }
    usleep(300000);

    // 1) Missing payload
    http_request_test("GET", "/", NULL, NULL);

    // 2) payload with type: Create
    const char *y1 = "type: Create\nuser: alice";
    char enc1[MAX_INPUT_SIZE + 256];
    url_encode(y1, enc1, sizeof(enc1));
    char path1[4096];
    snprintf(path1, sizeof(path1), "/?payload=%s", enc1);
    http_request_test("GET", path1, NULL, NULL);

    // 3) payload with type: Update
    const char *y2 = "type: Update\nid: 42";
    char enc2[MAX_INPUT_SIZE + 256];
    url_encode(y2, enc2, sizeof(enc2));
    char path2[4096];
    snprintf(path2, sizeof(path2), "/?payload=%s", enc2);
    http_request_test("GET", path2, NULL, NULL);

    // 4) invalid YAML
    const char *y3 = "type Create\n";
    char enc3[MAX_INPUT_SIZE + 256];
    url_encode(y3, enc3, sizeof(enc3));
    char path3[4096];
    snprintf(path3, sizeof(path3), "/?payload=%s", enc3);
    http_request_test("GET", path3, NULL, NULL);

    // 5) too large payload via POST
    char *big = (char *)malloc(70010);
    if (!big) return 1;
    memset(big, 'A', 70000);
    big[70000] = '\0';
    char *y4 = (char *)malloc(70030);
    sprintf(y4, "type: Update\npad: %s", big);
    char *form = (char *)malloc(strlen(y4) * 3 + 32);
    char enc4[MAX_INPUT_SIZE + 10];
    url_encode(y4, form, strlen(y4) * 3 + 32);
    char *post = (char *)malloc(strlen(form) + 16);
    sprintf(post, "payload=%s", form);
    http_request_test("POST", "/", post, "application/x-www-form-urlencoded");
    free(big);
    free(y4);
    free(form);
    free(post);

    g_stop = 1;
    pthread_join(tid, NULL);
    return 0;
}