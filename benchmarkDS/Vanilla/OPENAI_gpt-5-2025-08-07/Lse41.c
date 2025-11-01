#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* html_escape(const char* s) {
    if (!s) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    size_t len = 0;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break;   // &amp;
            case '<': len += 4; break;   // &lt;
            case '>': len += 4; break;   // &gt;
            case '"': len += 6; break;   // &quot;
            case '\'': len += 5; break;  // &#39;
            default: len += 1; break;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    char* o = out;
    for (const char* p = s; *p; ++p) {
        switch (*p) {
            case '&': memcpy(o, "&amp;", 5); o += 5; break;
            case '<': memcpy(o, "&lt;", 4); o += 4; break;
            case '>': memcpy(o, "&gt;", 4); o += 4; break;
            case '"': memcpy(o, "&quot;", 6); o += 6; break;
            case '\'': memcpy(o, "&#39;", 5); o += 5; break;
            default: *o++ = *p; break;
        }
    }
    *o = '\0';
    return out;
}

static char* last4_digits(const char* ssn) {
    if (!ssn) {
        char* e = (char*)malloc(1);
        if (e) e[0] = '\0';
        return e;
    }
    char digits[128];
    size_t dlen = 0;
    for (const char* p = ssn; *p && dlen < sizeof(digits) - 1; ++p) {
        if (isdigit((unsigned char)*p)) {
            digits[dlen++] = *p;
        }
    }
    digits[dlen] = '\0';
    const char* start = digits;
    if (dlen > 4) start = digits + dlen - 4;
    size_t outlen = dlen > 4 ? 4 : dlen;
    char* out = (char*)malloc(outlen + 1);
    if (!out) return NULL;
    memcpy(out, start, outlen);
    out[outlen] = '\0';
    return out;
}

static char* render_info(const char* username, const char* email, const char* ssn) {
    char* u = html_escape(username ? username : "");
    char* e = html_escape(email ? email : "");
    char* l4_raw = last4_digits(ssn ? ssn : "");
    char* l4 = html_escape(l4_raw ? l4_raw : "");
    free(l4_raw);

    const char* head = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Info</title></head><body><h1>User Info</h1><ul><li>Username: ";
    const char* mid1 = "</li><li>Email: ";
    const char* mid2 = "</li><li>SSN Last4: ";
    const char* tail = "</li></ul></body></html>";

    size_t total = strlen(head) + strlen(u) + strlen(mid1) + strlen(e) + strlen(mid2) + strlen(l4) + strlen(tail);
    char* html = (char*)malloc(total + 1);
    if (!html) {
        free(u); free(e); free(l4);
        return NULL;
    }
    strcpy(html, head);
    strcat(html, u);
    strcat(html, mid1);
    strcat(html, e);
    strcat(html, mid2);
    strcat(html, l4);
    strcat(html, tail);

    free(u); free(e); free(l4);
    return html;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static char* url_decode(const char* in) {
    if (!in) {
        char* e = (char*)malloc(1);
        if (e) e[0] = '\0';
        return e;
    }
    size_t len = strlen(in);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = in[i];
        if (c == '+') {
            out[oi++] = ' ';
        } else if (c == '%' && i + 2 < len) {
            int h1 = hexval(in[i + 1]);
            int h2 = hexval(in[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                out[oi++] = (char)((h1 << 4) | h2);
                i += 2;
            } else {
                out[oi++] = c;
            }
        } else {
            out[oi++] = c;
        }
    }
    out[oi] = '\0';
    return out;
}

static void run_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8083);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }

    if (listen(server_fd, 16) < 0) {
        close(server_fd);
        return;
    }

    char buffer[8192];

    while (1) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) continue;

        ssize_t n = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(client);
            continue;
        }
        buffer[0 ? 0 : 0] = buffer[0]; // no-op to avoid warnings
        buffer[n] = '\0';

        // Parse request line
        char* line_end = strstr(buffer, "\r\n");
        if (!line_end) {
            close(client);
            continue;
        }
        size_t reqline_len = (size_t)(line_end - buffer);
        char reqline[1024];
        if (reqline_len >= sizeof(reqline)) reqline_len = sizeof(reqline) - 1;
        memcpy(reqline, buffer, reqline_len);
        reqline[reqline_len] = '\0';

        char method[16], target[1024];
        method[0] = target[0] = '\0';
        sscanf(reqline, "%15s %1023s", method, target);

        if (strcmp(method, "GET") != 0) {
            const char* resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
            send(client, resp, strlen(resp), 0);
            close(client);
            continue;
        }

        char path[1024] = {0};
        char query[2048] = {0};
        char* qpos = strchr(target, '?');
        if (qpos) {
            size_t pathlen = (size_t)(qpos - target);
            if (pathlen >= sizeof(path)) pathlen = sizeof(path) - 1;
            memcpy(path, target, pathlen);
            path[pathlen] = '\0';
            strncpy(query, qpos + 1, sizeof(query) - 1);
        } else {
            strncpy(path, target, sizeof(path) - 1);
        }

        if (strcmp(path, "/info") != 0) {
            const char* body = "<h1>Not Found</h1>";
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 404 Not Found\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     strlen(body));
            send(client, header, strlen(header), 0);
            send(client, body, strlen(body), 0);
            close(client);
            continue;
        }

        char *username = NULL, *email = NULL, *ssn = NULL;
        // Parse query string
        char* qcopy = (char*)malloc(strlen(query) + 1);
        if (!qcopy) {
            close(client);
            continue;
        }
        strcpy(qcopy, query);
        char* saveptr = NULL;
        char* token = strtok_r(qcopy, "&", &saveptr);
        while (token) {
            char* eq = strchr(token, '=');
            if (eq) {
                *eq = '\0';
                char* key_enc = token;
                char* val_enc = eq + 1;
                char* key = url_decode(key_enc);
                char* val = url_decode(val_enc);
                if (strcmp(key, "username") == 0) {
                    free(username);
                    username = val;
                } else if (strcmp(key, "email") == 0) {
                    free(email);
                    email = val;
                } else if (strcmp(key, "ssn") == 0) {
                    free(ssn);
                    ssn = val;
                } else {
                    free(val);
                }
                free(key);
            }
            token = strtok_r(NULL, "&", &saveptr);
        }
        free(qcopy);

        char* body = render_info(username ? username : "", email ? email : "", ssn ? ssn : "");
        size_t body_len = body ? strlen(body) : 0;

        char header[256];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                 body_len);
        send(client, header, strlen(header), 0);
        if (body && body_len > 0) send(client, body, body_len, 0);

        free(username);
        free(email);
        free(ssn);
        free(body);
        close(client);
    }

    close(server_fd);
}

int main() {
    // 5 test cases
    char* out1 = render_info("alice", "alice@example.com", "123-45-6789");
    char* out2 = render_info("bob", "bob@example.com", "000112223");
    char* out3 = render_info("charlie", "charlie@company.org", "9999");
    char* out4 = render_info("dora", "dora@mail.net", "111-22-3333");
    char* out5 = render_info("eve", "eve@domain.com", "12-34-56-78-90");
    if (out1) { printf("%s\n", out1); free(out1); }
    if (out2) { printf("%s\n", out2); free(out2); }
    if (out3) { printf("%s\n", out3); free(out3); }
    if (out4) { printf("%s\n", out4); free(out4); }
    if (out5) { printf("%s\n", out5); free(out5); }

    run_server();
    return 0;
}