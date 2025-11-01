#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct Session {
    char sid[33];
    char username[64];
    char email[128];
    char ssn[32];
    struct Session* next;
} Session;

Session* SESSIONS_C = NULL;

char* info(Session* s) {
    const char* u = s ? s->username : "";
    const char* e = s ? s->email : "";
    const char* ssn = s ? s->ssn : "";
    size_t len = strlen(u) + strlen(e) + strlen(ssn) + 32 + 32 + 32; // buffer headroom
    char* out = (char*)malloc(len);
    if (!out) return NULL;
    snprintf(out, len, "username: %s; email: %s; ssn: %s", u, e, ssn);
    return out;
}

void generate_sid(char out[33]) {
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        out[i] = hex[rand() % 16];
    }
    out[32] = '\0';
}

Session* find_session(const char* sid) {
    Session* cur = SESSIONS_C;
    while (cur) {
        if (strcmp(cur->sid, sid) == 0) return cur;
        cur = cur->next;
    }
    return NULL;
}

Session* create_session(const char* sid_in) {
    Session* s = (Session*)calloc(1, sizeof(Session));
    if (!s) return NULL;
    if (sid_in && sid_in[0]) {
        strncpy(s->sid, sid_in, 32);
        s->sid[32] = '\0';
    } else {
        generate_sid(s->sid);
    }
    strncpy(s->username, "guest", sizeof(s->username) - 1);
    strncpy(s->email, "guest@example.com", sizeof(s->email) - 1);
    strncpy(s->ssn, "000-00-0000", sizeof(s->ssn) - 1);
    s->next = SESSIONS_C;
    SESSIONS_C = s;
    return s;
}

char* parse_cookie_sid(const char* cookie) {
    if (!cookie) return NULL;
    const char* p = strstr(cookie, "SID=");
    if (!p) return NULL;
    p += 4; // skip "SID="
    const char* end = strchr(p, ';');
    size_t len = end ? (size_t)(end - p) : strlen(p);
    char* sid = (char*)malloc(len + 1);
    if (!sid) return NULL;
    memcpy(sid, p, len);
    sid[len] = '\0';
    // trim spaces
    while (len > 0 && (sid[0] == ' ' || sid[0] == '\t')) {
        memmove(sid, sid + 1, len);
        len--;
    }
    while (len > 0 && (sid[len - 1] == ' ' || sid[len - 1] == '\t')) {
        sid[len - 1] = '\0';
        len--;
    }
    return sid;
}

void run_server_c(unsigned short port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(srv); return; }
    if (listen(srv, 16) < 0) { close(srv); return; }
    printf("C server running on http://localhost:%u/info\n", port);
    fflush(stdout);

    while (1) {
        int c = accept(srv, NULL, NULL);
        if (c < 0) continue;
        char buf[4096];
        ssize_t n = recv(c, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(c); continue; }
        buf[n] = '\0';

        // parse request line
        char* line_end = strstr(buf, "\r\n");
        if (!line_end) { close(c); continue; }
        *line_end = '\0';
        if (strncmp(buf, "GET /info", 9) != 0) {
            const char* resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(c, resp, strlen(resp), 0);
            close(c);
            continue;
        }

        // parse headers
        char* headers = line_end + 2;
        char* cookieHeader = NULL;
        char* p = headers;
        while (1) {
            char* next = strstr(p, "\r\n");
            if (!next || next == p) break;
            *next = '\0';
            if (strncasecmp(p, "Cookie:", 7) == 0) {
                cookieHeader = p + 7;
                while (*cookieHeader == ' ' || *cookieHeader == '\t') cookieHeader++;
            }
            p = next + 2;
        }

        char* sid = parse_cookie_sid(cookieHeader);
        int isNew = 0;
        Session* s = NULL;
        if (sid) s = find_session(sid);
        if (!s) {
            if (!sid) {
                char tmp[33]; generate_sid(tmp);
                s = create_session(tmp);
                sid = strdup(s->sid);
            } else {
                s = create_session(sid);
            }
            isNew = 1;
        }

        char* body = info(s);
        if (!body) {
            const char* resp = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
            send(c, resp, strlen(resp), 0);
            close(c);
            free(sid);
            continue;
        }

        char header[512];
        int header_len = 0;
        if (isNew) {
            header_len = snprintf(header, sizeof(header),
                                  "HTTP/1.1 200 OK\r\n"
                                  "Set-Cookie: SID=%s; Path=/; HttpOnly\r\n"
                                  "Content-Type: text/plain; charset=utf-8\r\n"
                                  "Content-Length: %zu\r\n"
                                  "Connection: close\r\n\r\n",
                                  sid, strlen(body));
        } else {
            header_len = snprintf(header, sizeof(header),
                                  "HTTP/1.1 200 OK\r\n"
                                  "Content-Type: text/plain; charset=utf-8\r\n"
                                  "Content-Length: %zu\r\n"
                                  "Connection: close\r\n\r\n",
                                  strlen(body));
        }
        send(c, header, header_len, 0);
        send(c, body, strlen(body), 0);
        close(c);
        free(body);
        free(sid);
    }
}

int main() {
    srand((unsigned int)time(NULL));
    // 5 test cases
    Session t1 = {0}, t2 = {0}, t3 = {0}, t4 = {0}, t5 = {0};
    strcpy(t1.username, "alice"); strcpy(t1.email, "alice@example.com"); strcpy(t1.ssn, "111-22-3333");
    strcpy(t2.username, "bob"); strcpy(t2.email, "bob@example.com"); strcpy(t2.ssn, "222-33-4444");
    strcpy(t3.username, "carol"); strcpy(t3.email, "carol@example.com"); strcpy(t3.ssn, "333-44-5555");
    strcpy(t4.username, "dave"); strcpy(t4.email, "dave@example.com"); strcpy(t4.ssn, "444-55-6666");
    strcpy(t5.username, "eve"); strcpy(t5.email, "eve@example.com"); strcpy(t5.ssn, "555-66-7777");

    char* o1 = info(&t1); printf("Test 1: %s\n", o1); free(o1);
    char* o2 = info(&t2); printf("Test 2: %s\n", o2); free(o2);
    char* o3 = info(&t3); printf("Test 3: %s\n", o3); free(o3);
    char* o4 = info(&t4); printf("Test 4: %s\n", o4); free(o4);
    char* o5 = info(&t5); printf("Test 5: %s\n", o5); free(o5);

    run_server_c(8083);
    return 0;
}