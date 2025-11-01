#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

bool matches_allowed(const char* target) {
    static int compiled = 0;
    static regex_t re;
    if (!compiled) {
        if (regcomp(&re, ".*example\\.com.*", REG_EXTENDED | REG_NOSUB) != 0) {
            return false;
        }
        compiled = 1;
    }
    if (target == NULL) return false;
    return regexec(&re, target, 0, NULL, 0) == 0;
}

char* resolve_redirect(const char* target) {
    if (target == NULL) target = "";
    if (matches_allowed(target)) {
        char* out = (char*)malloc(strlen(target) + 1);
        strcpy(out, target);
        return out;
    } else {
        char* out = (char*)malloc(2);
        strcpy(out, "/");
        return out;
    }
}

char hex_to_char(const char* p) {
    char buf[3] = {p[0], p[1], 0};
    return (char)strtol(buf, NULL, 16);
}

char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len) {
            out[oi++] = hex_to_char(&s[i + 1]);
            i += 2;
        } else if (s[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = 0;
    return out;
}

char* get_query_param(const char* query, const char* key) {
    size_t keylen = strlen(key);
    const char* p = query;
    while (p && *p) {
        const char* amp = strchr(p, '&');
        size_t seglen = amp ? (size_t)(amp - p) : strlen(p);
        const char* eq = memchr(p, '=', seglen);
        size_t klen = eq ? (size_t)(eq - p) : seglen;
        if (klen == keylen && strncmp(p, key, keylen) == 0) {
            if (eq) {
                size_t vlen = seglen - (klen + 1);
                char* v = (char*)malloc(vlen + 1);
                memcpy(v, eq + 1, vlen);
                v[vlen] = 0;
                char* dec = url_decode(v);
                free(v);
                return dec;
            } else {
                char* empty = (char*)malloc(1);
                empty[0] = 0;
                return empty;
            }
        }
        p = amp ? amp + 1 : NULL;
    }
    return NULL;
}

void send_response(int client, const char* status, const char* headers, const char* body) {
    char* resp;
    size_t len = strlen("HTTP/1.1 ") + strlen(status) + 2 + strlen(headers) + 2 + (body ? strlen(body) : 0);
    resp = (char*)malloc(len + 1);
    sprintf(resp, "HTTP/1.1 %s\r\n%s\r\n%s", status, headers, body ? body : "");
    send(client, resp, strlen(resp), 0);
    free(resp);
}

void handle_client(int client) {
    char buf[4096];
    ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
    if (n <= 0) {
        close(client);
        return;
    }
    buf[n] = 0;
    char* line_end = strstr(buf, "\r\n");
    if (!line_end) line_end = strchr(buf, '\n');
    if (!line_end) {
        close(client);
        return;
    }
    char line[1024];
    size_t linelen = (size_t)(line_end - buf);
    if (linelen > sizeof(line) - 1) linelen = sizeof(line) - 1;
    memcpy(line, buf, linelen);
    line[linelen] = 0;

    char method[16], target[1024], version[16];
    method[0] = target[0] = version[0] = 0;
    sscanf(line, "%15s %1023s %15s", method, target, version);

    if (strcmp(method, "GET") != 0) {
        send_response(client, "405 Method Not Allowed", "Content-Length: 0\r\nConnection: close", "");
        close(client);
        return;
    }

    char path[1024];
    char query[1024];
    path[0] = 0;
    query[0] = 0;
    char* qpos = strchr(target, '?');
    if (qpos) {
        size_t plen = (size_t)(qpos - target);
        if (plen > sizeof(path) - 1) plen = sizeof(path) - 1;
        memcpy(path, target, plen);
        path[plen] = 0;
        strncpy(query, qpos + 1, sizeof(query) - 1);
        query[sizeof(query) - 1] = 0;
    } else {
        strncpy(path, target, sizeof(path) - 1);
        path[sizeof(path) - 1] = 0;
        query[0] = 0;
    }

    if (strcmp(path, "/go") == 0) {
        char* t = get_query_param(query, "target");
        char* loc = resolve_redirect(t);
        free(t);
        char headers[2048];
        snprintf(headers, sizeof(headers), "Location: %s\r\nContent-Length: 0\r\nConnection: close", loc);
        send_response(client, "302 Found", headers, "");
        free(loc);
    } else if (strcmp(path, "/") == 0) {
        const char* body = "Home";
        char headers[256];
        snprintf(headers, sizeof(headers), "Content-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close", strlen(body));
        send_response(client, "200 OK", headers, body);
    } else {
        const char* body = "Not Found";
        char headers[256];
        snprintf(headers, sizeof(headers), "Content-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close", strlen(body));
        send_response(client, "404 Not Found", headers, body);
    }

    close(client);
}

struct ServerArgs {
    int port;
    int max_requests;
};

void* server_thread(void* arg) {
    struct ServerArgs* sa = (struct ServerArgs*)arg;
    int port = sa->port;
    int max_requests = sa->max_requests;

    int s = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);
    bind(s, (struct sockaddr*)&addr, sizeof(addr));
    listen(s, 16);
    for (int i = 0; i < max_requests; ++i) {
        int client = accept(s, NULL, NULL);
        if (client >= 0) {
            handle_client(client);
        }
    }
    close(s);
    return NULL;
}

void http_get_no_follow(const char* host, int port, const char* path, int* out_status, char* out_location, size_t loc_size) {
    *out_status = 0;
    out_location[0] = 0;
    int c = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);
    connect(c, (struct sockaddr*)&addr, sizeof(addr));
    char req[2048];
    snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    send(c, req, strlen(req), 0);
    char buf[4096];
    ssize_t n;
    size_t total = 0;
    char* resp = NULL;
    while ((n = recv(c, buf, sizeof(buf), 0)) > 0) {
        resp = (char*)realloc(resp, total + n + 1);
        memcpy(resp + total, buf, n);
        total += n;
        resp[total] = 0;
    }
    close(c);
    if (resp) {
        char* sp1 = strchr(resp, ' ');
        if (sp1) {
            char* sp2 = strchr(sp1 + 1, ' ');
            if (sp2) {
                char codebuf[4] = {0};
                size_t clen = (size_t)(sp2 - (sp1 + 1));
                if (clen < sizeof(codebuf)) {
                    memcpy(codebuf, sp1 + 1, clen);
                    *out_status = atoi(codebuf);
                }
            }
        }
        const char* needle = "\r\nLocation: ";
        char* pos = strstr(resp, needle);
        if (pos) {
            pos += strlen(needle);
            char* end = strstr(pos, "\r\n");
            size_t l = end ? (size_t)(end - pos) : strlen(pos);
            if (l >= loc_size) l = loc_size - 1;
            memcpy(out_location, pos, l);
            out_location[l] = 0;
        }
        free(resp);
    }
}

char* url_encode(const char* s) {
    static const char* hex = "0123456789ABCDEF";
    size_t len = strlen(s);
    char* out = (char*)malloc(len * 3 + 1);
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char ch = (unsigned char)s[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' || ch == '.' || ch == '~') {
            out[oi++] = ch;
        } else {
            out[oi++] = '%';
            out[oi++] = hex[(ch >> 4) & 0xF];
            out[oi++] = hex[ch & 0xF];
        }
    }
    out[oi] = 0;
    return out;
}

int main() {
    int port = 19082;
    int requests = 5;
    pthread_t th;
    struct ServerArgs sa = {port, requests};
    pthread_create(&th, NULL, server_thread, &sa);
    usleep(200 * 1000);

    const char* tests[5] = {
        "http://example.com/page",
        "https://sub.example.com",
        "http://evil.com/?next=example.com",
        "https://another.com/path",
        NULL
    };
    for (int i = 0; i < 5; ++i) {
        char path[2048];
        if (tests[i] == NULL) {
            snprintf(path, sizeof(path), "/go");
        } else {
            char* enc = url_encode(tests[i]);
            snprintf(path, sizeof(path), "/go?target=%s", enc);
            free(enc);
        }
        int status;
        char location[1024];
        http_get_no_follow("127.0.0.1", port, path, &status, location, sizeof(location));
        printf("Test %d: target=%s -> redirect to %s\n", i + 1, tests[i] ? tests[i] : "(missing)", location[0] ? location : "(none)");
    }

    pthread_join(th, NULL);
    return 0;
}