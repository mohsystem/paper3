#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sqlite3.h>
#include <pthread.h>

int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    regex_t regex;
    int reti = regcomp(&regex, "^[A-Za-z0-9_]{1,32}$", REG_EXTENDED | REG_NOSUB);
    if (reti) return 0;
    reti = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    return reti == 0;
}

// Accepts input as parameters and returns output (1 true, 0 false)
int is_moderator(sqlite3* db, const char* username) {
    if (!db || !is_valid_username(username)) return 0;
    const char* sql = "SELECT is_moderator FROM users WHERE username = ?";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    int result = 0;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int val = sqlite3_column_int(stmt, 0);
        result = (val == 1) ? 1 : 0;
    }
    sqlite3_finalize(stmt);
    return result;
}

char hex2char(const char* p) {
    int v = 0;
    sscanf(p, "%2x", (unsigned int*)&v);
    return (char)v;
}

void urldecode(const char* src, char* dst, size_t dstsz) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 1 < dstsz; ++i) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            dst[j++] = hex2char(&src[i+1]);
            i += 2;
        } else if (src[i] == '+') {
            dst[j++] = ' ';
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

void get_query_param(const char* query, const char* key, char* out, size_t outsz) {
    out[0] = '\0';
    size_t keylen = strlen(key);
    const char* p = query;
    while (p && *p) {
        const char* amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        if (len > keylen + 1 && strncmp(p, key, keylen) == 0 && p[keylen] == '=') {
            char tmp[1024];
            size_t vlen = len - keylen - 1;
            if (vlen >= sizeof(tmp)) vlen = sizeof(tmp) - 1;
            memcpy(tmp, p + keylen + 1, vlen);
            tmp[vlen] = '\0';
            urldecode(tmp, out, outsz);
            return;
        }
        if (!amp) break;
        p = amp + 1;
    }
}

typedef struct {
    sqlite3* db;
    int port;
} server_args_t;

void* server_thread(void* arg) {
    server_args_t* args = (server_args_t*)arg;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return NULL;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(args->port);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(sfd); return NULL; }
    if (listen(sfd, 8) < 0) { close(sfd); return NULL; }

    for (int i = 0; i < 5; ++i) {
        int cfd = accept(sfd, NULL, NULL);
        if (cfd < 0) continue;
        char buf[2048];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        // Parse request line
        char method[8] = {0}, path[1024] = {0}, version[16] = {0};
        sscanf(buf, "%7s %1023s %15s", method, path, version);

        int status = 200;
        char body[128] = {0};
        snprintf(body, sizeof(body), "{\"isModerator\":false}");

        if (strcmp(method, "GET") == 0) {
            char route[1024] = {0};
            char query[1024] = {0};
            char* qmark = strchr(path, '?');
            if (qmark) {
                size_t rlen = (size_t)(qmark - path);
                if (rlen >= sizeof(route)) rlen = sizeof(route) - 1;
                memcpy(route, path, rlen);
                route[rlen] = '\0';
                snprintf(query, sizeof(query), "%s", qmark + 1);
            } else {
                snprintf(route, sizeof(route), "%s", path);
                query[0] = '\0';
            }
            if (strcmp(route, "/is_moderator") == 0) {
                char user[128];
                get_query_param(query, "user", user, sizeof(user));
                int res = is_moderator(args->db, user);
                snprintf(body, sizeof(body), "{\"isModerator\":%s}", res ? "true" : "false");
            } else {
                status = 404;
                snprintf(body, sizeof(body), "{\"error\":\"Not Found\"}");
            }
        } else {
            status = 405;
            snprintf(body, sizeof(body), "{\"error\":\"Method Not Allowed\"}");
        }

        char header[512];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 %d OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                 status, strlen(body));
        send(cfd, header, strlen(header), 0);
        send(cfd, body, strlen(body), 0);
        close(cfd);
    }
    close(sfd);
    return NULL;
}

void init_db(sqlite3* db) {
    sqlite3_exec(db,
                 "CREATE TABLE IF NOT EXISTS users("
                 " username TEXT PRIMARY KEY,"
                 " is_moderator INTEGER NOT NULL CHECK(is_moderator IN (0,1))"
                 ");", NULL, NULL, NULL);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('alice',1)", NULL, NULL, NULL);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('bob',0)", NULL, NULL, NULL);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('charlie',0)", NULL, NULL, NULL);
    sqlite3_exec(db, "INSERT OR REPLACE INTO users(username,is_moderator) VALUES('admin_1',1)", NULL, NULL, NULL);
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "DB open failed\n");
        return 1;
    }
    init_db(db);

    server_args_t args;
    args.db = db;
    args.port = 8082;

    pthread_t tid;
    pthread_create(&tid, NULL, server_thread, &args);

    // 5 test cases using direct calls
    const char* tests[5] = {"alice", "bob", "charlie", "admin_1", "unknown_user"};
    for (int i = 0; i < 5; ++i) {
        int r = is_moderator(db, tests[i]);
        printf("%s -> %s\n", tests[i], r ? "true" : "false");
    }

    pthread_join(tid, NULL);
    sqlite3_close(db);
    return 0;
}