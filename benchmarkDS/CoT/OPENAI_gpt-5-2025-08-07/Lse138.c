#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Connect to the database and initialize schema/data
sqlite3* connect_and_init_db() {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        return NULL;
    }
    const char* create_sql = "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT, is_moderator INTEGER);";
    const char* insert_sql =
        "INSERT INTO users (id, username, is_moderator) VALUES "
        "(1,'alice',1),(2,'bob',0),(3,'carol',1),(4,'dave',0),(5,'eve',0);";
    char* err = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(err);
        sqlite3_close(db);
        return NULL;
    }
    if (sqlite3_exec(db, insert_sql, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(err);
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

// Execute a query to check if the user is a moderator
int is_moderator(sqlite3* db, long user_id) {
    const char* sql = "SELECT is_moderator FROM users WHERE id = ?";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_int64(stmt, 1, (sqlite3_int64)user_id);
    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int v = sqlite3_column_int(stmt, 0);
        result = (v == 1) ? 1 : 0;
    }
    sqlite3_finalize(stmt);
    return result;
}

static void url_decode(char* s) {
    char* o = s;
    for (; *s; s++, o++) {
        if (*s == '%' && s[1] && s[2]) {
            char hex[3] = { s[1], s[2], 0 };
            *o = (char)strtol(hex, NULL, 16);
            s += 2;
        } else if (*s == '+') {
            *o = ' ';
        } else {
            *o = *s;
        }
    }
    *o = '\0';
}

static int get_query_param(const char* query, const char* key, char* out, size_t outlen) {
    size_t keylen = strlen(key);
    const char* p = query;
    while (p && *p) {
        const char* amp = strchr(p, '&');
        size_t seglen = amp ? (size_t)(amp - p) : strlen(p);
        if (seglen >= keylen + 1 && strncmp(p, key, keylen) == 0 && p[keylen] == '=') {
            size_t val_len = seglen - keylen - 1;
            if (val_len >= outlen) val_len = outlen - 1;
            memcpy(out, p + keylen + 1, val_len);
            out[val_len] = '\0';
            url_decode(out);
            return 1;
        }
        p = amp ? amp + 1 : NULL;
    }
    return 0;
}

// Define a simple web app route and server
void run_server(sqlite3* db, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return;
    }
    if (listen(fd, 16) < 0) {
        close(fd);
        return;
    }

    int max_conn = 5;
    while (max_conn-- > 0) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd < 0) continue;
        char buf[2048];
        ssize_t n = read(cfd, buf, sizeof(buf) - 1);
        if (n <= 0) {
            close(cfd);
            continue;
        }
        buf[n] = '\0';

        char method[16], path[1024], version[16];
        method[0] = path[0] = version[0] = '\0';
        sscanf(buf, "%15s %1023s %15s", method, path, version);

        int status = 404;
        const char* body = "Not Found";
        char resp[256];

        if (strcmp(method, "GET") == 0) {
            char* qmark = strchr(path, '?');
            char route[1024];
            if (qmark) {
                size_t rlen = (size_t)(qmark - path);
                if (rlen >= sizeof(route)) rlen = sizeof(route) - 1;
                memcpy(route, path, rlen);
                route[rlen] = '\0';
            } else {
                strncpy(route, path, sizeof(route) - 1);
                route[sizeof(route) - 1] = '\0';
            }

            if (strcmp(route, "/isModerator") == 0) {
                char uid_str[64] = {0};
                const char* query = qmark ? qmark + 1 : "";
                if (get_query_param(query, "userId", uid_str, sizeof(uid_str))) {
                    long uid = strtol(uid_str, NULL, 10);
                    int res = is_moderator(db, uid);
                    body = res ? "true" : "false";
                    status = 200;
                } else {
                    body = "false";
                    status = 200;
                }
            }
        }

        int blen = (int)strlen(body);
        int hlen = snprintf(resp, sizeof(resp),
                            "HTTP/1.1 %d OK\r\n"
                            "Content-Type: text/plain; charset=utf-8\r\n"
                            "Content-Length: %d\r\n"
                            "Connection: close\r\n\r\n",
                            status, blen);
        write(cfd, resp, (size_t)hlen);
        write(cfd, body, (size_t)blen);
        close(cfd);
    }
    close(fd);
}

int main() {
    sqlite3* db = connect_and_init_db();
    if (!db) {
        fprintf(stderr, "Failed to init DB\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        run_server(db, 8083);
        sqlite3_close(db);
        exit(0);
    } else if (pid > 0) {
        printf("C server started on http://localhost:8083/isModerator?userId={id}\n");
        // 5 test cases
        long ids[5] = {1, 2, 3, 4, 999};
        for (int i = 0; i < 5; ++i) {
            int res = is_moderator(db, ids[i]);
            printf("Test userId=%ld -> isModerator=%s\n", ids[i], res ? "true" : "false");
        }
        sleep(3);
    } else {
        fprintf(stderr, "Failed to fork\n");
    }

    sqlite3_close(db);
    return 0;
}