#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    char username[64];
    char email[128];
    char ssn[32];
} Session;

static void sanitize_user(const char* in, char* out, size_t outsz) {
    if (!in || !*in) { snprintf(out, outsz, "unknown"); return; }
    size_t j=0;
    for (size_t i=0; in[i] && j+1<outsz; ++i) {
        char c = in[i];
        if (isalnum((unsigned char)c) || c=='-' || c=='_' || c=='.') out[j++] = c;
    }
    if (j==0) { snprintf(out, outsz, "unknown"); return; }
    out[j]='\0';
}

static void sanitize_email(const char* in, char* out, size_t outsz) {
    if (!in || !*in) { snprintf(out, outsz, "unknown@example.com"); return; }
    size_t j=0;
    int has_at=0;
    for (size_t i=0; in[i] && j+1<outsz; ++i) {
        char c = in[i];
        if (isalnum((unsigned char)c) || c=='-' || c=='_' || c=='.' || c=='@') {
            out[j++] = c;
            if (c=='@') has_at=1;
        }
    }
    out[j]='\0';
    if (!has_at) snprintf(out, outsz, "unknown@example.com");
}

static void mask_ssn(const char* in, char* out, size_t outsz) {
    if (!in || !*in) { snprintf(out, outsz, "*********"); return; }
    int digits=0;
    for (size_t i=0; in[i]; ++i) if (isdigit((unsigned char)in[i])) digits++;
    int keep = digits < 4 ? digits : 4;
    int keep_from = digits - keep;
    int di=0;
    size_t j=0;
    for (size_t i=0; in[i] && j+1<outsz; ++i) {
        char c = in[i];
        if (isdigit((unsigned char)c)) {
            if (di < keep_from) out[j++]='*';
            else out[j++]=c;
            di++;
        } else {
            out[j++]=c;
        }
    }
    out[j]='\0';
}

char* info(const Session* s) {
    if (!s) return strdup("username: unknown; email: unknown@example.com; ssn: ********");
    char u[64], e[128], m[32];
    sanitize_user(s->username, u, sizeof(u));
    sanitize_email(s->email, e, sizeof(e));
    mask_ssn(s->ssn, m, sizeof(m));
    char* out = (char*)malloc(64 + 128 + 32 + 30);
    if (!out) return NULL;
    snprintf(out, 64+128+32+30, "username: %s; email: %s; ssn: %s", u, e, m);
    return out;
}

static void print_test(const Session* s) {
    char* r = info(s);
    if (r) { printf("%s\n", r); free(r); }
}

static char* parse_cookie_sid(const char* req) {
    const char* p = strstr(req, "Cookie:");
    if (!p) return NULL;
    const char* eol = strstr(p, "\r\n");
    size_t len = eol ? (size_t)(eol - p) : strlen(p);
    char* line = (char*)malloc(len + 1);
    if (!line) return NULL;
    memcpy(line, p, len); line[len]='\0';
    char* sid = NULL;
    char* token = strtok(line+7, ";"); // skip 'Cookie:'
    while (token) {
        while (*token==' ') token++;
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            if (strcmp(token, "SID")==0) {
                char* v = eq+1;
                while (*v==' ') v++;
                sid = strdup(v);
                break;
            }
        }
        token = strtok(NULL, ";");
    }
    free(line);
    return sid;
}

typedef struct {
    char sid[33];
    Session sess;
    int used;
} SidEntry;

#define MAX_SESS 64
static SidEntry store[MAX_SESS];

static SidEntry* find_or_create_sid(const char* sid) {
    if (sid) {
        for (int i=0;i<MAX_SESS;i++) {
            if (store[i].used && strncmp(store[i].sid, sid, 32)==0) return &store[i];
        }
    }
    for (int i=0;i<MAX_SESS;i++) {
        if (!store[i].used) {
            // generate sid
            static const char* hex = "0123456789abcdef";
            for (int j=0;j<32;j++) store[i].sid[j]=hex[rand()%16];
            store[i].sid[32]='\0';
            store[i].used=1;
            snprintf(store[i].sess.username, sizeof(store[i].sess.username), "demo_user");
            snprintf(store[i].sess.email, sizeof(store[i].sess.email), "demo_user@example.com");
            snprintf(store[i].sess.ssn, sizeof(store[i].sess.ssn), "123-45-6789");
            return &store[i];
        }
    }
    return NULL;
}

int main() {
    srand(12345);

    // 5 test cases
    Session s1 = { "alice", "alice@example.com", "123-45-6789" };
    print_test(&s1);
    Session s2 = { "bob_123", "bob-123@domain.co.uk", "987654321" };
    print_test(&s2);
    Session s3 = { "charlie!", "charlie_at_example.com", "111-22-3333" };
    print_test(&s3);
    Session s4 = { "dora.d", "dora.d@example.com", "000000000" };
    print_test(&s4);
    Session s5 = { "", "", "" };
    print_test(&s5);

    // Simple HTTP server on 127.0.0.1:8082
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return 1;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(8082);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(srv); return 1; }
    if (listen(srv, 8) < 0) { close(srv); return 1; }

    for (;;) {
        int c = accept(srv, NULL, NULL);
        if (c < 0) continue;
        char buf[4096];
        ssize_t n = recv(c, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(c); continue; }
        buf[n]='\0';
        if (strncmp(buf, "GET /info", 9)==0) {
            char* sid = parse_cookie_sid(buf);
            SidEntry* ent = find_or_create_sid(sid);
            free(sid);
            char* body = info(&ent->sess);
            if (!body) { close(c); continue; }

            char header[512];
            int set_cookie_len = 0;
            char set_cookie[128] = {0};
            // Always set cookie to ensure SID present
            set_cookie_len = snprintf(set_cookie, sizeof(set_cookie), "Set-Cookie: SID=%s; HttpOnly; SameSite=Lax; Path=/\r\n", ent->sid);

            int hlen = snprintf(header, sizeof(header),
                                "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain; charset=utf-8\r\n"
                                "X-Content-Type-Options: nosniff\r\n"
                                "Cache-Control: no-store\r\n"
                                "%s"
                                "Content-Length: %zu\r\n"
                                "\r\n",
                                set_cookie, strlen(body));
            send(c, header, hlen, 0);
            send(c, body, strlen(body), 0);
            free(body);
        } else {
            const char* nf = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(c, nf, strlen(nf), 0);
        }
        close(c);
    }
    close(srv);
    return 0;
}