#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME 50
#define MAX_MESSAGE 500
#define MAX_BODY 4096
#define MAX_MESSAGES 100

typedef struct {
    char username[MAX_USERNAME + 1];
    char message[MAX_MESSAGE + 1];
} Record;

static Record DB[MAX_MESSAGES];
static int DB_COUNT = 0;

static void safe_append(char* dst, size_t dstsz, const char* fmt, ...) {
    size_t len = strnlen(dst, dstsz);
    if (len >= dstsz) return;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(dst + len, dstsz - len, fmt, ap);
    va_end(ap);
}

static void html_escape(const char* in, char* out, size_t outsz) {
    out[0] = '\0';
    for (size_t i = 0; in[i] && strnlen(out, outsz) + 6 < outsz; i++) {
        char c = in[i];
        switch (c) {
            case '&': safe_append(out, outsz, "%s", "&amp;"); break;
            case '<': safe_append(out, outsz, "%s", "&lt;"); break;
            case '>': safe_append(out, outsz, "%s", "&gt;"); break;
            case '"': safe_append(out, outsz, "%s", "&quot;"); break;
            case '\'': safe_append(out, outsz, "%s", "&#x27;"); break;
            case '/': safe_append(out, outsz, "%s", "&#x2F;"); break;
            default: {
                char tmp[2] = {c, 0};
                safe_append(out, outsz, "%s", tmp);
            }
        }
    }
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static void url_decode(const char* in, char* out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] && oi + 1 < outsz; i++) {
        if (in[i] == '+') {
            out[oi++] = ' ';
        } else if (in[i] == '%' && in[i+1] && in[i+2]) {
            int h1 = hexval(in[i+1]);
            int h2 = hexval(in[i+2]);
            if (h1 >= 0 && h2 >= 0) {
                out[oi++] = (char)((h1 << 4) | h2);
                i += 2;
            } else {
                out[oi++] = '%';
            }
        } else {
            out[oi++] = in[i];
        }
    }
    out[oi] = '\0';
}

static bool insert_user_message_in_db(const char* username, const char* message) {
    if (!username || !message) return false;
    char u[MAX_USERNAME + 1]; char m[MAX_MESSAGE + 1];
    // trim
    const char *us = username, *ue = username + strlen(username);
    while (*us && isspace((unsigned char)*us)) us++;
    while (ue > us && isspace((unsigned char)ue[-1])) ue--;
    size_t ulen = (size_t)(ue - us);
    if (ulen > MAX_USERNAME) return false;
    memcpy(u, us, ulen); u[ulen] = '\0';

    const char *ms = message, *me = message + strlen(message);
    while (*ms && isspace((unsigned char)*ms)) ms++;
    while (me > ms && isspace((unsigned char)me[-1])) me--;
    size_t mlen = (size_t)(me - ms);
    if (mlen > MAX_MESSAGE) return false;
    memcpy(m, ms, mlen); m[mlen] = '\0';

    if (u[0] == '\0' || m[0] == '\0') return false;
    if (DB_COUNT >= MAX_MESSAGES) return false;
    snprintf(DB[DB_COUNT].username, sizeof(DB[DB_COUNT].username), "%s", u);
    snprintf(DB[DB_COUNT].message, sizeof(DB[DB_COUNT].message), "%s", m);
    DB_COUNT++;
    return true;
}

static void render_messages_html(char* out, size_t outsz) {
    out[0] = '\0';
    safe_append(out, outsz, "<!doctype html><html><head><meta charset='utf-8'>");
    safe_append(out, outsz, "<meta http-equiv='Content-Security-Policy' content=\"default-src 'self'; style-src 'self' 'unsafe-inline'\">");
    safe_append(out, outsz, "<title>Messages</title></head><body><h1>Messages</h1>");
    safe_append(out, outsz, "<form method='POST' action='/post'>");
    safe_append(out, outsz, "<label>Username: <input name='username' maxlength='%d'></label><br>", MAX_USERNAME);
    safe_append(out, outsz, "<label>Message: <input name='message' maxlength='%d'></label><br>", MAX_MESSAGE);
    safe_append(out, outsz, "<button type='submit'>Post</button></form><hr><ul>");
    for (int i = 0; i < DB_COUNT; i++) {
        char eu[(MAX_USERNAME*6)+16]; char em[(MAX_MESSAGE*6)+16];
        html_escape(DB[i].username, eu, sizeof(eu));
        html_escape(DB[i].message, em, sizeof(em));
        safe_append(out, outsz, "<li><strong>%s</strong>: %s</li>", eu, em);
    }
    safe_append(out, outsz, "</ul></body></html>");
}

static bool recv_until(int fd, char** out, size_t* out_len, const char* delim, size_t max_len) {
    size_t cap = 8192;
    char* buf = (char*)malloc(cap);
    if (!buf) return false;
    size_t len = 0;
    size_t delim_len = strlen(delim);
    while (1) {
        if (len + 1024 > cap) {
            if (cap > max_len) { free(buf); return false; }
            size_t ncap = cap * 2;
            if (ncap > max_len) ncap = max_len;
            char* nbuf = (char*)realloc(buf, ncap);
            if (!nbuf) { free(buf); return false; }
            cap = ncap; buf = nbuf;
        }
        ssize_t n = recv(fd, buf + len, 1024, 0);
        if (n <= 0) { free(buf); return false; }
        len += (size_t)n;
        if (len >= delim_len) {
            for (size_t i = 0; i + delim_len <= len; i++) {
                if (memcmp(buf + i, delim, delim_len) == 0) {
                    *out = buf; *out_len = len;
                    return true;
                }
            }
        }
        if (len >= max_len) { free(buf); return false; }
    }
}

int main(void) {
    // 5 test cases (pre-populate)
    insert_user_message_in_db("alice", "Hello from C!");
    insert_user_message_in_db("bob", "Keep inputs bounded.");
    insert_user_message_in_db("charlie", "No <b>bold</b> please.");
    insert_user_message_in_db("dora", "Encode & escape.");
    insert_user_message_in_db("eve", "Stay safe.");

    signal(SIGPIPE, SIG_IGN);
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return 1;
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(sfd); return 1; }
    if (listen(sfd, 16) < 0) { close(sfd); return 1; }

    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;

        char* req = NULL; size_t req_len = 0;
        if (!recv_until(cfd, &req, &req_len, "\r\n\r\n", 8192)) { close(cfd); continue; }
        // Null-terminate for ease
        req = (char*)realloc(req, req_len + 1);
        req[req_len] = '\0';

        // Parse request line
        char *line_end = strstr(req, "\r\n");
        if (!line_end) { free(req); close(cfd); continue; }
        *line_end = '\0';
        char method[8], path[256], version[16];
        method[0]=path[0]=version[0]='\0';
        sscanf(req, "%7s %255s %15s", method, path, version);

        // Parse headers
        char *headers = line_end + 2;
        char *body_start = strstr(headers, "\r\n\r\n");
        size_t header_len = body_start ? (size_t)(body_start - req) + 4 : req_len;
        char *body = body_start ? body_start + 4 : req + req_len;
        size_t body_size = req_len > header_len ? req_len - header_len : 0;

        // Find Content-Length and Content-Type
        size_t content_length = 0;
        char content_type[128]; content_type[0]='\0';
        char *hpos = headers;
        while (hpos && hpos < req + header_len - 2) {
            char *hend = strstr(hpos, "\r\n");
            if (!hend) break;
            *hend = '\0';
            char key[64], val[256]; key[0]=val[0]='\0';
            if (sscanf(hpos, "%63[^:]: %255[^\r\n]", key, val) == 2) {
                // lowercase key
                for (char* p=key; *p; ++p) *p = (char)tolower((unsigned char)*p);
                if (strcmp(key, "content-length") == 0) {
                    content_length = (size_t)strtoul(val, NULL, 10);
                    if (content_length > MAX_BODY) content_length = MAX_BODY;
                } else if (strcmp(key, "content-type") == 0) {
                    snprintf(content_type, sizeof(content_type), "%s", val);
                }
            }
            hpos = hend + 2;
        }

        // Read remaining body if needed
        if (content_length > body_size) {
            size_t need = content_length - body_size;
            char *nbuf = (char*)malloc(content_length + 1);
            if (!nbuf) { free(req); close(cfd); continue; }
            memcpy(nbuf, body, body_size);
            size_t got = body_size;
            while (got < content_length) {
                ssize_t n = recv(cfd, nbuf + got, content_length - got, 0);
                if (n <= 0) break;
                got += (size_t)n;
            }
            nbuf[(got < content_length) ? got : content_length] = '\0';
            body = nbuf;
            body_size = got;
        }

        if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
            char page[65536];
            render_messages_html(page, sizeof(page));
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     strlen(page));
            send(cfd, header, strlen(header), 0);
            send(cfd, page, strlen(page), 0);
        } else if (strcmp(method, "POST") == 0 && strcmp(path, "/post") == 0) {
            // Check content-type
            for (char* p=content_type; *p; ++p) *p = (char)tolower((unsigned char)*p);
            if (strncmp(content_type, "application/x-www-form-urlencoded", 33) != 0) {
                const char* resp = "HTTP/1.1 415 Unsupported Media Type\r\nConnection: close\r\n\r\n";
                send(cfd, resp, strlen(resp), 0);
            } else {
                char *body_copy = (char*)malloc(body_size + 1);
                if (!body_copy) {
                    const char* resp = "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n";
                    send(cfd, resp, strlen(resp), 0);
                } else {
                    memcpy(body_copy, body, body_size);
                    body_copy[body_size] = '\0';
                    // Parse form
                    char *saveptr = NULL;
                    char *tok = strtok_r(body_copy, "&", &saveptr);
                    char username_raw[256]="", message_raw[1024]="";
                    while (tok) {
                        char *eq = strchr(tok, '=');
                        if (eq) {
                            *eq = '\0';
                            char key[256]; char val[2048];
                            url_decode(tok, key, sizeof(key));
                            url_decode(eq+1, val, sizeof(val));
                            if (strcmp(key, "username") == 0) {
                                snprintf(username_raw, sizeof(username_raw), "%s", val);
                            } else if (strcmp(key, "message") == 0) {
                                snprintf(message_raw, sizeof(message_raw), "%s", val);
                            }
                        }
                        tok = strtok_r(NULL, "&", &saveptr);
                    }
                    insert_user_message_in_db(username_raw, message_raw);
                    free(body_copy);
                    const char* resp = "HTTP/1.1 303 See Other\r\nLocation: /\r\nConnection: close\r\n\r\n";
                    send(cfd, resp, strlen(resp), 0);
                }
            }
        } else {
            const char* resp = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
            send(cfd, resp, strlen(resp), 0);
        }

        if (body != req + req_len) free(body);
        free(req);
        close(cfd);
    }

    close(sfd);
    return 0;
}