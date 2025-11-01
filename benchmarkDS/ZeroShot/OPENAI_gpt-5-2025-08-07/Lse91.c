#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define PORT_C 8083
#define MAX_BODY 2048
#define ITERATIONS 100000
#define SALT_LEN 16
#define KEY_LEN 32
static const char* DB_PATH_C = "users_c.db";

int init_db(const char* db_path) {
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    const char* sql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 50) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

int is_valid_password(const char* p) {
    if (!p) return 0;
    size_t len = strlen(p);
    if (len < 8 || len > 200) return 0;
    int hasLetter = 0, hasDigit = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)p[i];
        if (isalpha(c)) hasLetter = 1;
        if (isdigit(c)) hasDigit = 1;
    }
    return hasLetter && hasDigit;
}

char* to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hex[data[i] >> 4];
        out[2*i + 1] = hex[data[i] & 0x0F];
    }
    out[len * 2] = '\0';
    return out;
}

int hash_password(const char* password, char** salt_hex, char** hash_hex) {
    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) return 0;
    unsigned char out[KEY_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, ITERATIONS, EVP_sha256(), KEY_LEN, out) != 1) {
        return 0;
    }
    *salt_hex = to_hex(salt, SALT_LEN);
    *hash_hex = to_hex(out, KEY_LEN);
    OPENSSL_cleanse(out, KEY_LEN);
    return *salt_hex && *hash_hex;
}

char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '+') out[oi++] = ' ';
        else if (s[i] == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hex[3] = {s[i+1], s[i+2], '\0'};
            out[oi++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = '\0';
    return out;
}

void parse_form(const char* body, char** username, char** password) {
    *username = NULL; *password = NULL;
    char* dup = strdup(body);
    if (!dup) return;
    char* saveptr = NULL;
    for (char* part = strtok_r(dup, "&", &saveptr); part != NULL; part = strtok_r(NULL, "&", &saveptr)) {
        char* eq = strchr(part, '=');
        if (eq) {
            *eq = '\0';
            char* k = url_decode(part);
            char* v = url_decode(eq + 1);
            if (k && strcmp(k, "username") == 0) {
                *username = v;
            } else if (k && strcmp(k, "password") == 0) {
                *password = v;
            } else {
                free(v);
            }
            if (k) free(k);
        }
    }
    free(dup);
}

char* register_user(const char* db_path, const char* username, const char* password) {
    if (!is_valid_username(username)) return strdup("ERR: invalid username");
    if (!is_valid_password(password)) return strdup("ERR: weak password");
    char* salt_hex = NULL;
    char* hash_hex = NULL;
    if (!hash_password(password, &salt_hex, &hash_hex)) {
        if (salt_hex) free(salt_hex);
        if (hash_hex) free(hash_hex);
        return strdup("ERR: hashing failed");
    }
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        free(salt_hex); free(hash_hex);
        return strdup("ERR: database error");
    }
    const char* sql = "INSERT INTO users(username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        free(salt_hex); free(hash_hex);
        return strdup("ERR: database error");
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hash_hex, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, salt_hex, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(salt_hex); free(hash_hex);
    if (rc == SQLITE_DONE) return strdup("OK: registered");
    if (rc == SQLITE_CONSTRAINT) return strdup("ERR: username exists");
    return strdup("ERR: database error");
}

void send_response(int client, int code, const char* text) {
    char header[256];
    int len = (int)strlen(text);
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d \r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
                     code, len);
    send(client, header, n, 0);
    send(client, text, len, 0);
}

void* server_thread(void* arg) {
    (void)arg;
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return NULL;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_C);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(srv); return NULL; }
    if (listen(srv, 10) < 0) { close(srv); return NULL; }
    printf("C server started on http://127.0.0.1:%d\n", PORT_C);
    char buf[4096];
    while (1) {
        int client = accept(srv, NULL, NULL);
        if (client < 0) continue;
        ssize_t n = recv(client, buf, sizeof(buf)-1, 0);
        if (n <= 0) { close(client); continue; }
        buf[n] = '\0';
        if (strncmp(buf, "POST /register ", 15) != 0) {
            send_response(client, 404, "Not Found");
            close(client);
            continue;
        }
        char* headers_end = strstr(buf, "\r\n\r\n");
        if (!headers_end) {
            send_response(client, 400, "Bad Request");
            close(client);
            continue;
        }
        int content_length = -1;
        char* p = buf;
        while (p < headers_end) {
            char* line_end = strstr(p, "\r\n");
            if (!line_end) break;
            if (strncasecmp(p, "Content-Length:", 15) == 0) {
                p += 15;
                while (p < line_end && isspace((unsigned char)*p)) p++;
                content_length = atoi(p);
            }
            p = line_end + 2;
        }
        if (content_length < 0 || content_length > MAX_BODY) {
            send_response(client, 413, "Payload Too Large");
            close(client);
            continue;
        }
        char* body = headers_end + 4;
        int already = (int) (buf + n - body);
        char* payload = (char*)malloc((size_t)content_length + 1);
        if (!payload) { send_response(client, 500, "Internal Server Error"); close(client); continue; }
        int copied = 0;
        if (already > 0) {
            int cpy = already > content_length ? content_length : already;
            memcpy(payload, body, (size_t)cpy);
            copied = cpy;
        }
        while (copied < content_length) {
            ssize_t m = recv(client, payload + copied, (size_t)(content_length - copied), 0);
            if (m <= 0) break;
            copied += (int)m;
        }
        if (copied < content_length) {
            free(payload);
            send_response(client, 400, "Bad Request");
            close(client);
            continue;
        }
        payload[content_length] = '\0';
        char* username = NULL; char* password = NULL;
        parse_form(payload, &username, &password);
        free(payload);
        if (!username || !password) {
            if (username) free(username);
            if (password) free(password);
            send_response(client, 400, "Bad Request");
            close(client);
            continue;
        }
        char* res = register_user(DB_PATH_C, username, password);
        if (strncmp(res, "OK", 2) == 0) send_response(client, 201, "Registered");
        else if (strstr(res, "exists")) send_response(client, 409, "Conflict");
        else send_response(client, 400, "Bad Request");
        free(username); free(password); free(res);
        close(client);
    }
    close(srv);
    return NULL;
}

int main() {
    if (!init_db(DB_PATH_C)) {
        fprintf(stderr, "DB init failed\n");
        return 1;
    }
    pthread_t th;
    pthread_create(&th, NULL, server_thread, NULL);
    pthread_detach(th);

    // 5 Test cases (direct function calls)
    char* r1 = register_user(DB_PATH_C, "alice_01", "StrongPass123!");
    printf("[Test1] %s\n", r1); free(r1);
    char* r2 = register_user(DB_PATH_C, "alice_01", "AnotherStrong123!");
    printf("[Test2] %s\n", r2); free(r2);
    char* r3 = register_user(DB_PATH_C, "bad user!", "StrongPass123!");
    printf("[Test3] %s\n", r3); free(r3);
    char* r4 = register_user(DB_PATH_C, "bob", "123");
    printf("[Test4] %s\n", r4); free(r4);
    char* r5 = register_user(DB_PATH_C, "charlie_02", "Y3tAn0ther$trong");
    printf("[Test5] %s\n", r5); free(r5);

    // Keep running
    while (1) sleep(60);
    return 0;
}