#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    char username[128];
    char email[256];
    char ssn[64];
    int has_ssn;
} Session;

typedef struct {
    char sid[129];
    Session session;
} KV;

static KV SESSIONS[8];
static size_t SESS_COUNT = 0;
static pthread_mutex_t SESS_MTX = PTHREAD_MUTEX_INITIALIZER;

static void safe_strcpy(char* dst, size_t dstsz, const char* src) {
    if (dstsz == 0) return;
    size_t n = 0;
    while (n + 1 < dstsz && src[n]) {
        dst[n] = src[n];
        n++;
    }
    dst[n] = '\0';
}

static char* format_info(const Session* s, char* out, size_t outsz) {
    if (!s->has_ssn || s->username[0] == '\0' || s->email[0] == '\0') return NULL;
    int n = snprintf(out, outsz, "username=%s; email=%s; ssn=%s", s->username, s->email, s->ssn);
    if (n < 0 || (size_t)n >= outsz) return NULL;
    return out;
}

static int is_valid_sid(const char* sid) {
    size_t len = sid ? strlen(sid) : 0;
    if (len < 8 || len > 128) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)sid[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return 0;
    }
    return 1;
}

static int find_session_by_sid(const char* sid, Session* out) {
    int found = 0;
    pthread_mutex_lock(&SESS_MTX);
    for (size_t i = 0; i < SESS_COUNT; i++) {
        if (strcmp(SESSIONS[i].sid, sid) == 0) {
            if (out) *out = SESSIONS[i].session;
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&SESS_MTX);
    return found;
}

static void send_response(int fd, int code, const char* body) {
    const char* status = "500 Internal Server Error";
    switch (code) {
        case 200: status = "200 OK"; break;
        case 400: status = "400 Bad Request"; break;
        case 401: status = "401 Unauthorized"; break;
        case 404: status = "404 Not Found"; break;
        case 405: status = "405 Method Not Allowed"; break;
    }
    if (!body) body = "";
    char header[512];
    int blen = (int)strlen(body);
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %s\r\n"
                     "Content-Type: text/plain; charset=UTF-8\r\n"
                     "Cache-Control: no-store\r\n"
                     "Content-Length: %d\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     status, blen);
    if (n < 0) return;
    ssize_t left = n;
    const char* p = header;
    while (left > 0) {
        ssize_t w = send(fd, p, (size_t)left, 0);
        if (w <= 0) break;
        p += w;
        left -= w;
    }
    left = blen;
    p = body;
    while (left > 0) {
        ssize_t w = send(fd, p, (size_t)left, 0);
        if (w <= 0) break;
        p += w;
        left -= w;
    }
}

static void* server_loop(void* arg) {
    uint16_t port = *(uint16_t*)arg;
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return NULL;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(srv);
        return NULL;
    }
    if (listen(srv, 16) < 0) {
        close(srv);
        return NULL;
    }

    for (;;) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(srv, &fds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000;
        int r = select(srv + 1, &fds, NULL, NULL, &tv);
        if (r <= 0) continue;
        if (!FD_ISSET(srv, &fds)) continue;

        struct sockaddr_in cli;
        socklen_t cl = sizeof(cli);
        int fd = accept(srv, (struct sockaddr*)&cli, &cl);
        if (fd < 0) continue;

        char req[8192];
        size_t used = 0;
        for (;;) {
            ssize_t n = recv(fd, req + used, sizeof(req) - used, 0);
            if (n <= 0) break;
            used += (size_t)n;
            if (used >= 4) {
                for (size_t i = 3; i < used; i++) {
                    if (req[i-3] == '\r' && req[i-2] == '\n' && req[i-1] == '\r' && req[i] == '\n') {
                        goto headers_done;
                    }
                }
            }
            if (used >= sizeof(req)) break;
        }
headers_done:
        if (used == 0) {
            send_response(fd, 400, "Bad Request");
            close(fd);
            continue;
        }
        req[used < sizeof(req) ? used : sizeof(req)-1] = '\0';

        // Parse request line
        char* line_end = strstr(req, "\r\n");
        if (!line_end) {
            send_response(fd, 400, "Bad Request");
            close(fd);
            continue;
        }
        *line_end = '\0';
        char method[8] = {0}, path[64] = {0}, version[16] = {0};
        if (sscanf(req, "%7s %63s %15s", method, path, version) != 3) {
            send_response(fd, 400, "Bad Request");
            close(fd);
            continue;
        }
        if (strcmp(method, "GET") != 0) {
            send_response(fd, 405, "Method Not Allowed");
            close(fd);
            continue;
        }
        if (strcmp(path, "/info") != 0) {
            send_response(fd, 404, "Not Found");
            close(fd);
            continue;
        }
        // Find Cookie header and extract SID
        char* headers_start = line_end + 2;
        char* sid = NULL;
        {
            char* p = headers_start;
            while (p && *p) {
                char* e = strstr(p, "\r\n");
                if (!e) break;
                *e = '\0';
                if (strncasecmp(p, "Cookie:", 7) == 0) {
                    char* v = p + 7;
                    while (*v == ' ' || *v == '\t') v++;
                    // Parse cookie pairs
                    char* saveptr = NULL;
                    for (char* tok = strtok_r(v, ";", &saveptr); tok; tok = strtok_r(NULL, ";", &saveptr)) {
                        while (*tok == ' ' || *tok == '\t') tok++;
                        char* eq = strchr(tok, '=');
                        if (!eq) continue;
                        *eq = '\0';
                        char* k = tok;
                        char* val = eq + 1;
                        // trim val
                        while (*val == ' ' || *val == '\t') val++;
                        // trim end
                        char* end = val + strlen(val);
                        while (end > val && (end[-1] == ' ' || end[-1] == '\t')) { end--; }
                        *end = '\0';
                        if (strcasecmp(k, "SID") == 0) {
                            if (is_valid_sid(val)) {
                                sid = val;
                            } else {
                                sid = NULL;
                            }
                            break;
                        }
                    }
                }
                *e = '\r'; // restore (not necessary but keeps buffer consistent)
                e[1] = '\n';
                p = e + 2;
                if (sid) break;
            }
        }
        if (!sid) {
            send_response(fd, 401, "Unauthorized");
            close(fd);
            continue;
        }
        Session s;
        if (!find_session_by_sid(sid, &s)) {
            send_response(fd, 401, "Unauthorized");
            close(fd);
            continue;
        }
        char body[512];
        char* b = format_info(&s, body, sizeof(body));
        if (!b) {
            send_response(fd, 400, "Bad Request");
            close(fd);
            continue;
        }
        send_response(fd, 200, b);
        close(fd);
    }
    return NULL;
}

static int http_test(const char* method, const char* host, uint16_t port, const char* path, const char* cookie, int expected_code, const char* expected_body_opt) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return 0;
    }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return 0;
    }
    char req[1024];
    int n = snprintf(req, sizeof(req),
                     "%s %s HTTP/1.1\r\n"
                     "Host: %s:%u\r\n"
                     "Connection: close\r\n"
                     "%s%s%s"
                     "\r\n",
                     method, path, host, (unsigned)port,
                     cookie ? "Cookie: " : "",
                     cookie ? cookie : "",
                     cookie ? "\r\n" : "");
    if (n < 0 || (size_t)n >= sizeof(req)) {
        close(fd);
        return 0;
    }
    ssize_t left = n;
    const char* p = req;
    while (left > 0) {
        ssize_t w = send(fd, p, (size_t)left, 0);
        if (w <= 0) break;
        p += w;
        left -= w;
    }
    char resp[8192];
    size_t used = 0;
    for (;;) {
        ssize_t r = recv(fd, resp + used, sizeof(resp) - used, 0);
        if (r <= 0) break;
        used += (size_t)r;
        if (used >= sizeof(resp)) break;
    }
    close(fd);
    if (used == 0) return 0;
    resp[used < sizeof(resp) ? used : sizeof(resp)-1] = '\0';
    // Parse status
    char* line_end = strstr(resp, "\r\n");
    if (!line_end) return 0;
    *line_end = '\0';
    int code = 0;
    char httpv[16];
    if (sscanf(resp, "%15s %d", httpv, &code) != 2) return 0;
    int status_ok = (code == expected_code);
    int body_ok = 1;
    if (expected_body_opt && code == 200) {
        char* hdr_end = strstr(line_end + 2, "\r\n\r\n");
        if (!hdr_end) return 0;
        char* body = hdr_end + 4;
        body_ok = (strcmp(body, expected_body_opt) == 0);
    }
    return status_ok && body_ok;
}

int main(void) {
    // Populate sessions
    const char* SID1 = "b7f1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1";
    const char* SID2 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    const char* SID3 = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    pthread_mutex_lock(&SESS_MTX);
    safe_strcpy(SESSIONS[SESS_COUNT].sid, sizeof(SESSIONS[SESS_COUNT].sid), SID1);
    safe_strcpy(SESSIONS[SESS_COUNT].session.username, sizeof(SESSIONS[SESS_COUNT].session.username), "alice");
    safe_strcpy(SESSIONS[SESS_COUNT].session.email, sizeof(SESSIONS[SESS_COUNT].session.email), "alice@example.com");
    safe_strcpy(SESSIONS[SESS_COUNT].session.ssn, sizeof(SESSIONS[SESS_COUNT].session.ssn), "123-45-6789");
    SESSIONS[SESS_COUNT].session.has_ssn = 1;
    SESS_COUNT++;

    safe_strcpy(SESSIONS[SESS_COUNT].sid, sizeof(SESSIONS[SESS_COUNT].sid), SID2);
    safe_strcpy(SESSIONS[SESS_COUNT].session.username, sizeof(SESSIONS[SESS_COUNT].session.username), "bob");
    safe_strcpy(SESSIONS[SESS_COUNT].session.email, sizeof(SESSIONS[SESS_COUNT].session.email), "bob@example.com");
    SESSIONS[SESS_COUNT].session.ssn[0] = '\0';
    SESSIONS[SESS_COUNT].session.has_ssn = 0; // missing ssn
    SESS_COUNT++;

    safe_strcpy(SESSIONS[SESS_COUNT].sid, sizeof(SESSIONS[SESS_COUNT].sid), SID3);
    safe_strcpy(SESSIONS[SESS_COUNT].session.username, sizeof(SESSIONS[SESS_COUNT].session.username), "carol");
    safe_strcpy(SESSIONS[SESS_COUNT].session.email, sizeof(SESSIONS[SESS_COUNT].session.email), "carol@example.com");
    safe_strcpy(SESSIONS[SESS_COUNT].session.ssn, sizeof(SESSIONS[SESS_COUNT].session.ssn), "987-65-4321");
    SESSIONS[SESS_COUNT].session.has_ssn = 1;
    SESS_COUNT++;
    pthread_mutex_unlock(&SESS_MTX);

    uint16_t port = 18115;
    pthread_t tid;
    if (pthread_create(&tid, NULL, server_loop, &port) != 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    // Give server time to start
    usleep(200000);

    int passed = 0, total = 5;
    const char* host = "127.0.0.1";
    const char* path = "/info";

    // Test 1: valid SID
    const char* expected1 = "username=alice; email=alice@example.com; ssn=123-45-6789";
    if (http_test("GET", host, port, path, "SID=b7f1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1", 200, expected1)) passed++;

    // Test 2: missing cookie
    if (http_test("GET", host, port, path, NULL, 401, NULL)) passed++;

    // Test 3: unknown SID
    if (http_test("GET", host, port, path, "SID=deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef", 401, NULL)) passed++;

    // Test 4: missing ssn
    if (http_test("GET", host, port, path, "SID=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 400, NULL)) passed++;

    // Test 5: wrong method
    if (http_test("POST", host, port, path, "SID=b7f1c2d3e4f5a6b7c8d9e0f1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1", 405, NULL)) passed++;

    printf("Tests passed: %d/%d\n", passed, total);

    // Terminate server thread (not gracefully since no stop flag; detach and exit)
    pthread_cancel(tid);
    pthread_join(tid, NULL);
    return 0;
}