#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PORT 8083

typedef struct SessionData {
    char username[128];
    char email[256];
    char ssn[64];
    char sid[65]; // 64 hex + null
    struct SessionData* next;
} SessionData;

static SessionData* sessions = NULL;

static void trim(char* s) {
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t')) start++;
    size_t end = len;
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\r' || s[end-1] == '\n')) end--;
    if (start > 0 || end < len) {
        memmove(s, s + start, end - start);
        s[end - start] = '\0';
    }
}

static void html_escape(const char* in, char* out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0' && j + 6 < outsz; i++) {
        char c = in[i];
        if ((unsigned char)c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
            continue;
        }
        switch (c) {
            case '&': if (j + 5 < outsz) { strcpy(out + j, "&amp;"); j += 5; } break;
            case '<': if (j + 4 < outsz) { strcpy(out + j, "&lt;"); j += 4; } break;
            case '>': if (j + 4 < outsz) { strcpy(out + j, "&gt;"); j += 4; } break;
            case '"': if (j + 6 < outsz) { strcpy(out + j, "&quot;"); j += 6; } break;
            case '\'': if (j + 6 < outsz) { strcpy(out + j, "&#x27;"); j += 6; } break;
            case '/': if (j + 6 < outsz) { strcpy(out + j, "&#x2F;"); j += 6; } break;
            default: out[j++] = c;
        }
    }
    out[j] = '\0';
}

static void last4_digits(const char* ssn, char* out, size_t outsz) {
    char digits[64];
    size_t d = 0;
    for (size_t i = 0; ssn && ssn[i] != '\0' && d < sizeof(digits)-1; i++) {
        if (ssn[i] >= '0' && ssn[i] <= '9') digits[d++] = ssn[i];
    }
    digits[d] = '\0';
    if (d < 4) {
        snprintf(out, outsz, "****");
    } else {
        snprintf(out, outsz, "%s", digits + (d - 4));
    }
}

// Core function: accepts inputs and returns the resulting HTML page in heap memory.
// Caller must free the returned pointer.
static char* render_info_page(const char* username, const char* email, const char* ssn) {
    char safeUser[512], safeEmail[512], l4[16];
    html_escape(username ? username : "", safeUser, sizeof(safeUser));
    html_escape(email ? email : "", safeEmail, sizeof(safeEmail));
    last4_digits(ssn ? ssn : "", l4, sizeof(l4));

    char now[128];
    time_t t = time(NULL);
    struct tm gmt;
    gmtime_r(&t, &gmt);
    strftime(now, sizeof(now), "%a, %d %b %Y %H:%M:%S GMT", &gmt);

    const char* tpl1 = "<!doctype html><html lang=\"en\"><head>"
                       "<meta charset=\"utf-8\">"
                       "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'self'\">"
                       "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                       "<title>Info</title>"
                       "</head><body><h1>Account Info</h1><ul>";
    const char* tpl2 = "</ul><p>Generated at ";
    const char* tpl3 = "</p></body></html>";

    size_t needed = strlen(tpl1) + strlen(tpl2) + strlen(tpl3)
                    + strlen(safeUser) + strlen(safeEmail) + strlen(l4)
                    + strlen(now) + 128;
    char* out = (char*)malloc(needed);
    if (!out) return NULL;
    snprintf(out, needed,
             "%s<li>Username: %s</li><li>Email: %s</li><li>SSN (last 4): %s</li>%s%s%s",
             tpl1, safeUser, safeEmail, l4, tpl2, now, tpl3);
    return out;
}

static void random_sid(char* out, size_t outsz) {
    // Generate 32 bytes and print as 64 hex chars
    unsigned char bytes[32];
    FILE* ur = fopen("/dev/urandom", "rb");
    if (ur) {
        fread(bytes, 1, sizeof(bytes), ur);
        fclose(ur);
    } else {
        for (size_t i = 0; i < sizeof(bytes); i++) bytes[i] = (unsigned char)rand();
    }
    const char* hex = "0123456789ABCDEF";
    size_t j = 0;
    for (size_t i = 0; i < sizeof(bytes) && j + 2 < outsz; i++) {
        out[j++] = hex[(bytes[i] >> 4) & 0xF];
        out[j++] = hex[bytes[i] & 0xF];
    }
    out[j] = '\0';
}

static SessionData* find_session(const char* sid) {
    for (SessionData* p = sessions; p; p = p->next) {
        if (strncmp(p->sid, sid, 64) == 0) return p;
    }
    return NULL;
}

static SessionData* create_session() {
    SessionData* s = (SessionData*)calloc(1, sizeof(SessionData));
    if (!s) return NULL;
    random_sid(s->sid, sizeof(s->sid));
    snprintf(s->username, sizeof(s->username), "demo_user");
    snprintf(s->email, sizeof(s->email), "demo.user@example.com");
    snprintf(s->ssn, sizeof(s->ssn), "111-22-3333");
    s->next = sessions;
    sessions = s;
    return s;
}

static void send_response(int client, int new_cookie, const char* sid, const char* body) {
    char header[1024];
    size_t body_len = body ? strlen(body) : 0;
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html; charset=utf-8\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "X-Frame-Options: DENY\r\n"
                     "Referrer-Policy: no-referrer\r\n"
                     "Cache-Control: no-store\r\n"
                     "%s"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     new_cookie ? "Set-Cookie: SID=%s; Path=/; HttpOnly; SameSite=Strict; Secure\r\n" : "",
                     body_len);
    if (new_cookie) {
        // If we included Set-Cookie placeholder, we must rebuild header with SID
        n = snprintf(header, sizeof(header),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html; charset=utf-8\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "X-Frame-Options: DENY\r\n"
                     "Referrer-Policy: no-referrer\r\n"
                     "Cache-Control: no-store\r\n"
                     "Set-Cookie: SID=%s; Path=/; HttpOnly; SameSite=Strict; Secure\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     sid, body_len);
    }
    send(client, header, (size_t)n, 0);
    if (body_len > 0) send(client, body, body_len, 0);
}

static void send_404(int client) {
    const char* body = "Not Found";
    char header[256];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 404 Not Found\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n", strlen(body));
    send(client, header, (size_t)n, 0);
    send(client, body, strlen(body), 0);
}

int main() {
    // 5 test cases for render_info_page
    char* t1 = render_info_page("alice", "alice@example.com", "123-45-6789");
    char* t2 = render_info_page("bob<script>", "bob@example.com", "000-00-0001");
    char* t3 = render_info_page("carol", "carol+test@example.com", "9999");
    char* t4 = render_info_page("dave", "dave.o'reilly@example.com", "12-3456");
    char* t5 = render_info_page("eve", "eve@example.com", "abcd-ef-ghij");
    if (t1) { printf("%s\n", t1); free(t1); }
    if (t2) { printf("%s\n", t2); free(t2); }
    if (t3) { printf("%s\n", t3); free(t3); }
    if (t4) { printf("%s\n", t4); free(t4); }
    if (t5) { printf("%s\n", t5); free(t5); }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }
    printf("C server started on http://127.0.0.1:%d/info\n", PORT);

    while (1) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) { perror("accept"); continue; }
        char buf[4096];
        ssize_t n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(client); continue; }
        buf[n] = '\0';

        if (strncmp(buf, "GET /info", 9) != 0 && strstr(buf, "GET /info ") == NULL) {
            send_404(client);
            close(client);
            continue;
        }

        // Parse Cookie header
        char* cookie_line = strcasestr(buf, "\nCookie:");
        char sid[65] = {0};
        if (cookie_line) {
            cookie_line += 8; // skip "\nCookie:"
            char* line_end = strstr(cookie_line, "\r\n");
            if (line_end) *line_end = '\0';
            trim(cookie_line);
            // parse SID
            char* tok = strtok(cookie_line, ";");
            while (tok) {
                trim(tok);
                if (strncasecmp(tok, "SID=", 4) == 0) {
                    strncpy(sid, tok + 4, sizeof(sid)-1);
                    break;
                }
                tok = strtok(NULL, ";");
            }
            if (line_end) *line_end = '\r'; // restore
        }

        int set_cookie = 0;
        if (strlen(sid) != 64) {
            SessionData* s = create_session();
            if (s) {
                strncpy(sid, s->sid, sizeof(sid)-1);
                set_cookie = 1;
            }
        }
        SessionData* sess = find_session(sid);
        if (!sess) {
            sess = create_session();
            if (sess) {
                strncpy(sid, sess->sid, sizeof(sid)-1);
                set_cookie = 1;
            }
        }

        char* body = NULL;
        if (sess) {
            body = render_info_page(sess->username, sess->email, sess->ssn);
        } else {
            body = strdup("Session not found");
        }
        if (!body) {
            body = strdup("Internal Server Error");
        }
        send_response(client, set_cookie, sid, body);
        free(body);
        close(client);
    }
    close(server_fd);
    return 0;
}