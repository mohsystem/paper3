#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static char* html_escape(const char* s) {
    if (!s) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t len = strlen(s);
    size_t extra = 0;
    for (size_t i = 0; i < len; i++) {
        switch (s[i]) {
            case '&': extra += 4; break;   // &amp; -> 5-1
            case '<': extra += 3; break;   // &lt; -> 4-1
            case '>': extra += 3; break;   // &gt; -> 4-1
            case '"': extra += 5; break;   // &quot; -> 6-1
            case '\'': extra += 6; break;  // &#x27; -> 6
            case '/': extra += 5; break;   // &#x2F; -> 5
            default: break;
        }
    }
    size_t outlen = len + extra + 1;
    char* out = (char*)malloc(outlen);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            case '/': rep = "&#x2F;"; break;
            default: break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            memcpy(out + j, rep, rlen);
            j += rlen;
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static bool is_valid_username(const char* u) {
    if (!u) return false;
    size_t len = strlen(u);
    if (len < 1 || len > 64) return false;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-' || c == '.')) return false;
    }
    return true;
}

static bool is_valid_email(const char* e) {
    if (!e) return false;
    size_t len = strlen(e);
    if (len < 5 || len > 254) return false;
    const char* at = strchr(e, '@');
    if (!at || strchr(at+1, '@') != NULL) return false;
    size_t local_len = (size_t)(at - e);
    const char* domain = at + 1;
    if (local_len == 0 || *domain == '\0') return false;
    if (domain[0] == '.' || domain[strlen(domain)-1] == '.') return false;
    if (strchr(domain, '.') == NULL) return false;
    for (size_t i = 0; i < local_len; i++) {
        char c = e[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-')) return false;
    }
    for (size_t i = 0; domain[i] != '\0'; i++) {
        char c = domain[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '-')) return false;
    }
    return true;
}

static char* extract_last4(const char* ssn) {
    if (!ssn) return NULL;
    char digits[32];
    size_t d = 0;
    for (size_t i = 0; ssn[i] != '\0' && d < sizeof(digits)-1; i++) {
        if (isdigit((unsigned char)ssn[i])) {
            digits[d++] = ssn[i];
        }
    }
    digits[d] = '\0';
    if (d < 4) return NULL;
    char* out = (char*)malloc(5);
    if (!out) return NULL;
    memcpy(out, digits + d - 4, 4);
    out[4] = '\0';
    return out;
}

static char* generate_info_page(const char* username, const char* email, const char* ssn) {
    if (!is_valid_username(username)) return NULL;
    if (!is_valid_email(email)) return NULL;
    char* last4 = extract_last4(ssn);
    if (!last4) return NULL;

    char* u = html_escape(username);
    char* e = html_escape(email);
    char* l4 = html_escape(last4);
    free(last4);
    if (!u || !e || !l4) {
        free(u); free(e); free(l4);
        return NULL;
    }

    const char* head = "<!doctype html><html><head><meta charset=\"utf-8\"><title>User Info</title></head><body><h1>User Info</h1><ul>";
    const char* tail = "</ul></body></html>";
    // Compute needed size conservatively
    size_t need = strlen(head) + strlen(tail) + strlen(u) + strlen(e) + strlen(l4) + 128;
    char* page = (char*)malloc(need);
    if (!page) {
        free(u); free(e); free(l4);
        return NULL;
    }
    int n = snprintf(page, need,
        "%s<li>Username: %s</li><li>Email: %s</li><li>SSN (last 4): %s</li>%s",
        head, u, e, l4, tail);
    free(u); free(e); free(l4);
    if (n < 0 || (size_t)n >= need) {
        free(page);
        return NULL;
    }
    return page;
}

static char from_hex(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'a' && c <= 'f') return (char)(10 + c - 'a');
    if (c >= 'A' && c <= 'F') return (char)(10 + c - 'A');
    return (char)-1;
}

static char* url_decode(const char* in) {
    size_t len = strlen(in);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = in[i];
        if (c == '%' && i + 2 < len) {
            char h1 = from_hex(in[i+1]);
            char h2 = from_hex(in[i+2]);
            if (h1 >= 0 && h2 >= 0) {
                out[j++] = (char)((h1 << 4) | h2);
                i += 2;
            } else {
                out[j++] = c;
            }
        } else if (c == '+') {
            out[j++] = ' ';
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

struct Query {
    char* username;
    char* email;
    char* ssn;
};

static void free_query(struct Query* q) {
    if (!q) return;
    free(q->username);
    free(q->email);
    free(q->ssn);
    q->username = q->email = q->ssn = NULL;
}

static struct Query parse_query(const char* q) {
    struct Query res = {0};
    if (!q) return res;
    const char* p = q;
    while (*p) {
        const char* amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        char* pair = (char*)malloc(len + 1);
        if (!pair) break;
        memcpy(pair, p, len); pair[len] = '\0';
        char* eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            char* key = url_decode(pair);
            char* val = url_decode(eq + 1);
            if (key && val) {
                if (strcmp(key, "username") == 0) { free(res.username); res.username = val; val = NULL; }
                else if (strcmp(key, "email") == 0) { free(res.email); res.email = val; val = NULL; }
                else if (strcmp(key, "ssn") == 0) { free(res.ssn); res.ssn = val; val = NULL; }
            }
            free(key);
            free(val);
        }
        free(pair);
        if (!amp) break;
        p = amp + 1;
    }
    return res;
}

static void send_response(int cfd, int code, const char* ctype, const char* body) {
    const char* msg = "Error";
    if (code == 200) msg = "OK";
    else if (code == 400) msg = "Bad Request";
    else if (code == 404) msg = "Not Found";
    else if (code == 405) msg = "Method Not Allowed";
    char hdr[512];
    size_t body_len = body ? strlen(body) : 0;
    int n = snprintf(hdr, sizeof(hdr),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "X-Frame-Options: DENY\r\n"
        "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        code, msg, ctype, body_len);
    if (n > 0) {
        send(cfd, hdr, (size_t)n, 0);
    }
    if (body_len > 0) {
        send(cfd, body, body_len, 0);
    }
}

static void run_tests(void) {
    struct {
        const char* u; const char* e; const char* s; int should_pass;
    } cases[5] = {
        {"alice_01","alice@example.com","123-45-6789",1},
        {"bob","bob@example.org","0000",1},
        {"charlie.z","charlie.z@example.co.uk","111223333",1},
        {"bad user","x@example.com","111-22-3333",0},
        {"dana","invalid","9999",0}
    };
    int passed = 0;
    for (int i=0;i<5;i++) {
        char* page = generate_info_page(cases[i].u, cases[i].e, cases[i].s);
        if (page && cases[i].should_pass) {
            // Check no script tag and last4 present
            char* p = page;
            for (; *p; ++p) *p = (char)tolower((unsigned char)*p);
            if (strstr(page, "<script") == NULL) {
                char* last4 = extract_last4(cases[i].s);
                if (last4 && strstr(page, last4)) {
                    passed++;
                }
                free(last4);
            }
            free(page);
        } else if (!page && !cases[i].should_pass) {
            passed++;
        } else {
            free(page);
        }
    }
    // printf("Tests passed: %d/5\n", passed);
}

static void serve(unsigned short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return; }
    if (listen(fd, 16) < 0) { close(fd); return; }

    char buf[4096];
    for (;;) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd < 0) continue;
        ssize_t n = recv(cfd, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        // parse first line
        char* line_end = strstr(buf, "\r\n");
        if (!line_end) { send_response(cfd, 400, "text/plain; charset=UTF-8", "Bad Request"); close(cfd); continue; }
        *line_end = '\0';
        char method[8], target[2048], version[16];
        method[0] = target[0] = version[0] = '\0';
        if (sscanf(buf, "%7s %2047s %15s", method, target, version) != 3) {
            send_response(cfd, 400, "text/plain; charset=UTF-8", "Bad Request"); close(cfd); continue;
        }
        if (strcmp(method, "GET") != 0) {
            send_response(cfd, 405, "text/plain; charset=UTF-8", "Method Not Allowed"); close(cfd); continue;
        }
        char path[2048];
        char* qmark = strchr(target, '?');
        if (qmark) {
            size_t plen = (size_t)(qmark - target);
            if (plen >= sizeof(path)) plen = sizeof(path)-1;
            memcpy(path, target, plen); path[plen] = '\0';
        } else {
            strncpy(path, target, sizeof(path)-1);
            path[sizeof(path)-1] = '\0';
        }
        if (strcmp(path, "/info") != 0) {
            send_response(cfd, 404, "text/plain; charset=UTF-8", "Not Found"); close(cfd); continue;
        }
        const char* q = qmark ? qmark + 1 : NULL;
        struct Query qp = parse_query(q ? q : "");
        if (!qp.username || !qp.email || !qp.ssn) {
            send_response(cfd, 400, "text/plain; charset=UTF-8", "Missing parameters");
            free_query(&qp);
            close(cfd);
            continue;
        }
        char* page = generate_info_page(qp.username, qp.email, qp.ssn);
        if (!page) {
            send_response(cfd, 400, "text/plain; charset=UTF-8", "Invalid input");
        } else {
            send_response(cfd, 200, "text/html; charset=UTF-8", page);
        }
        free(page);
        free_query(&qp);
        close(cfd);
    }
    close(fd);
}

int main(void) {
    run_tests();
    serve(8080);
    return 0;
}