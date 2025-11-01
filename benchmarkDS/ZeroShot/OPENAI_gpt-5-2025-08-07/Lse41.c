#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Simple safe string utilities
static char *html_escape(const char *s) {
    if (!s) s = "";
    size_t len = strlen(s);
    // Worst case every char becomes 6 chars
    size_t cap = len * 6 + 1;
    char *out = (char *)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; s[i]; i++) {
        unsigned char c = (unsigned char)s[i];
        const char *rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#39;"; break;
            default:
                if (c < 32) { out[j++] = ' '; rep = NULL; }
                else { out[j++] = c; rep = NULL; }
        }
        if (rep) {
            size_t rl = strlen(rep);
            memcpy(out + j, rep, rl);
            j += rl;
        }
        if (j + 7 >= cap) {
            cap *= 2;
            char *tmp = (char *)realloc(out, cap);
            if (!tmp) { free(out); return NULL; }
            out = tmp;
        }
    }
    out[j] = '\0';
    return out;
}

static char *safe_truncate(const char *s, size_t maxlen) {
    if (!s) s = "";
    size_t n = strnlen(s, maxlen);
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

static char *extract_last4(const char *ssn) {
    if (!ssn) return NULL;
    char buf[5] = {0};
    int count = 0;
    for (int i = (int)strlen(ssn) - 1; i >= 0 && count < 4; --i) {
        if (isdigit((unsigned char)ssn[i])) {
            buf[3 - count] = ssn[i];
            count++;
        }
    }
    if (count < 4) return NULL;
    char *out = (char *)malloc(5);
    if (!out) return NULL;
    memcpy(out, buf, 5);
    return out;
}

static bool is_valid_email(const char *email) {
    if (!email) return false;
    size_t len = strlen(email);
    if (len < 3 || len > 256) return false;
    const char *at = strchr(email, '@');
    if (!at || at == email || at == email + len - 1) return false;
    const char *dot = strchr(at + 1, '.');
    if (!dot || dot == at + 1 || dot == email + len - 1) return false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (c <= 31 || c == ' ' || c == '@') return false;
    }
    return true;
}

static char *html_page(const char *title, const char *body) {
    char *et = html_escape(title ? title : "");
    size_t blen = body ? strlen(body) : 0;
    size_t cap = strlen(et) + blen + 128;
    char *out = (char *)malloc(cap);
    if (!out) { free(et); return NULL; }
    snprintf(out, cap, "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>%s</title></head><body>%s</body></html>", et, body ? body : "");
    free(et);
    return out;
}

// Core function: builds the info page
char *build_info_page(const char *username, const char *email, const char *ssn) {
    char *u_tr = safe_truncate(username, 256);
    char *e_tr = safe_truncate(email, 256);
    char *u = html_escape(u_tr);
    char *e = html_escape(e_tr);
    char *last4 = extract_last4(ssn);
    char *out = NULL;

    if (!u || !e) goto cleanup;
    if (u[0] == '\0' || !is_valid_email(e_tr) || !last4) {
        out = html_page("Invalid input", "<p>Invalid or missing parameters.</p>");
        goto cleanup;
    }
    const char *prefix = "<h1>User Info</h1><ul><li>Username: ";
    const char *mid1 = "</li><li>Email: ";
    const char *mid2 = "</li><li>SSN (last 4): ";
    const char *suffix = "</li></ul>";
    size_t cap = strlen(prefix) + strlen(u) + strlen(mid1) + strlen(e) + strlen(mid2) + 4 + strlen(suffix) + 1;
    char *body = (char *)malloc(cap);
    if (!body) goto cleanup;
    snprintf(body, cap, "%s%s%s%s%s%s%s", prefix, u, mid1, e, mid2, last4, suffix);
    out = html_page("Info", body);
    free(body);

cleanup:
    free(u_tr);
    free(e_tr);
    free(u);
    free(e);
    if (last4) free(last4);
    return out;
}

// URL decode
static char from_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = (char)toupper((unsigned char)c);
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static char *url_decode(const char *s) {
    if (!s) return strdup("");
    size_t len = strlen(s);
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hi = from_hex(s[i+1]);
            char lo = from_hex(s[i+2]);
            out[j++] = (char)((hi << 4) | lo);
            i += 2;
        } else if (s[i] == '+') {
            out[j++] = ' ';
        } else {
            out[j++] = s[i];
        }
    }
    out[j] = '\0';
    return out;
}

struct kv { char *k; char *v; };

static struct kv *parse_query(const char *qs, size_t *count) {
    *count = 0;
    if (!qs) return NULL;
    char *cpy = strdup(qs);
    if (!cpy) return NULL;
    size_t cap = 8;
    struct kv *arr = (struct kv *)calloc(cap, sizeof(struct kv));
    if (!arr) { free(cpy); return NULL; }
    char *saveptr = NULL;
    char *tok = strtok_r(cpy, "&", &saveptr);
    while (tok) {
        char *eq = strchr(tok, '=');
        char *k = NULL; char *v = NULL;
        if (eq) {
            *eq = '\0';
            k = url_decode(tok);
            v = url_decode(eq + 1);
        } else {
            k = url_decode(tok);
            v = strdup("");
        }
        if (k && v) {
            if (*count >= cap) {
                cap *= 2;
                struct kv *tmp = (struct kv *)realloc(arr, cap * sizeof(struct kv));
                if (!tmp) { free(k); free(v); break; }
                arr = tmp;
            }
            arr[*count].k = k;
            arr[*count].v = v;
            (*count)++;
        } else {
            if (k) free(k);
            if (v) free(v);
        }
        tok = strtok_r(NULL, "&", &saveptr);
    }
    free(cpy);
    return arr;
}

static const char *get_param(struct kv *arr, size_t n, const char *key) {
    for (size_t i = 0; i < n; i++) {
        if (strcmp(arr[i].k, key) == 0) return arr[i].v;
    }
    return "";
}

static void free_kv(struct kv *arr, size_t n) {
    if (!arr) return;
    for (size_t i = 0; i < n; i++) {
        free(arr[i].k);
        free(arr[i].v);
    }
    free(arr);
}

struct client_arg { int fd; };

static void *handle_client(void *argp) {
    struct client_arg *arg = (struct client_arg *)argp;
    int cfd = arg->fd;
    free(arg);
    char buf[8192];
    ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) { close(cfd); return NULL; }
    buf[n] = '\0';
    // Parse request line
    char method[8] = {0}, target[4096] = {0}, version[16] = {0};
    sscanf(buf, "%7s %4095s %15s", method, target, version);
    if (strcmp(method, "GET") != 0) {
        const char *resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 18\r\nContent-Type: text/plain; charset=UTF-8\r\nX-Content-Type-Options: nosniff\r\n\r\nMethod Not Allowed";
        send(cfd, resp, strlen(resp), 0);
        close(cfd);
        return NULL;
    }
    char path[4096] = {0};
    char *qmark = strchr(target, '?');
    if (qmark) {
        size_t plen = (size_t)(qmark - target);
        if (plen >= sizeof(path)) plen = sizeof(path) - 1;
        memcpy(path, target, plen); path[plen] = '\0';
    } else {
        strncpy(path, target, sizeof(path)-1);
    }
    if (strcmp(path, "/info") != 0) {
        const char *body = "Not Found";
        char resp[256];
        int m = snprintf(resp, sizeof(resp),
                         "HTTP/1.1 404 Not Found\r\nContent-Length: %zu\r\nContent-Type: text/plain; charset=UTF-8\r\nX-Content-Type-Options: nosniff\r\n\r\n%s",
                         strlen(body), body);
        send(cfd, resp, (size_t)m, 0);
        close(cfd);
        return NULL;
    }
    const char *qs = qmark ? qmark + 1 : NULL;
    size_t cnt = 0;
    struct kv *params = parse_query(qs, &cnt);
    const char *username = get_param(params, cnt, "username");
    const char *email = get_param(params, cnt, "email");
    const char *ssn = get_param(params, cnt, "ssn");
    char *body = NULL;
    int status = 200;
    if (!username || !email || !ssn || username[0] == '\0' || email[0] == '\0' || ssn[0] == '\0') {
        body = html_page("Bad Request", "<p>Missing required parameters: username, email, ssn.</p>");
        status = 400;
    } else {
        body = build_info_page(username, email, ssn);
        if (strstr(body, "Invalid input") != NULL) status = 400;
    }
    if (!body) body = strdup("<!DOCTYPE html><html><body>Error</body></html>");
    char header[512];
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %s\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Security-Policy: default-src 'none'; style-src 'unsafe-inline'\r\nX-Content-Type-Options: nosniff\r\nContent-Length: %zu\r\n\r\n",
                        status == 200 ? "200 OK" : "400 Bad Request",
                        strlen(body));
    send(cfd, header, (size_t)hlen, 0);
    send(cfd, body, strlen(body), 0);
    free(body);
    free_kv(params, cnt);
    close(cfd);
    return NULL;
}

static void start_server(int port) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return;
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { close(sfd); return; }
    if (listen(sfd, 16) < 0) { close(sfd); return; }
    for (;;) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        pthread_t th;
        struct client_arg *arg = (struct client_arg *)malloc(sizeof(struct client_arg));
        if (!arg) { close(cfd); continue; }
        arg->fd = cfd;
        pthread_create(&th, NULL, handle_client, arg);
        pthread_detach(th);
    }
}

int main(void) {
    // 5 test cases for the core function
    char *p1 = build_info_page("alice", "alice@example.com", "123-45-6789");
    char *p2 = build_info_page("<bob>", "bob@example.org", "0000");
    char *p3 = build_info_page("charlie & co", "charlie@sub.example.com", "xxx-yy-9876");
    char *p4 = build_info_page("dora", "invalid-email", "111-22-3333");
    char *p5 = build_info_page("", "eve@example.net", "4444");
    if (p1) { printf("%s\n", p1); free(p1); }
    if (p2) { printf("%s\n", p2); free(p2); }
    if (p3) { printf("%s\n", p3); free(p3); }
    if (p4) { printf("%s\n", p4); free(p4); }
    if (p5) { printf("%s\n", p5); free(p5); }

    // Start server on port 8083
    start_server(8083);
    return 0;
}