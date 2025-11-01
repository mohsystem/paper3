#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define PBKDF2_ITERATIONS_C 100000
#define SALT_LEN_C 16
#define KEY_LEN_C 32

int init_db(sqlite3* db) {
    char* err = NULL;
    const char* sql1 = "PRAGMA foreign_keys=ON;";
    const char* sql2 = "CREATE TABLE IF NOT EXISTS users ("
                       "username TEXT PRIMARY KEY,"
                       "first_name TEXT NOT NULL,"
                       "last_name TEXT NOT NULL,"
                       "email TEXT NOT NULL,"
                       "password_hash TEXT NOT NULL,"
                       "salt BLOB NOT NULL,"
                       "iterations INTEGER NOT NULL,"
                       "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                       ");";
    const char* sql3 = "CREATE UNIQUE INDEX IF NOT EXISTS idx_users_username ON users(username);";
    if (sqlite3_exec(db, sql1, NULL, NULL, &err) != SQLITE_OK) { sqlite3_free(err); return 0; }
    if (sqlite3_exec(db, sql2, NULL, NULL, &err) != SQLITE_OK) { sqlite3_free(err); return 0; }
    if (sqlite3_exec(db, sql3, NULL, NULL, &err) != SQLITE_OK) { sqlite3_free(err); return 0; }
    return 1;
}

int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (!(isalnum((unsigned char)u[i]) || u[i] == '_')) return 0;
    }
    return 1;
}

int is_valid_password(const char* p) {
    if (!p) return 0;
    size_t len = strlen(p);
    return len >= 8 && len <= 1024;
}

int is_valid_email(const char* e) {
    if (!e) return 0;
    // Simple validation: must contain one '@' and a dot after '@'
    const char* at = strchr(e, '@');
    if (!at || at == e) return 0;
    const char* dot = strrchr(e, '.');
    if (!dot || dot < at + 2 || dot[1] == '\0') return 0;
    return 1;
}

static char* hex_encode(const unsigned char* data, size_t len) {
    static const char* hexdig = "0123456789abcdef";
    char* out = (char*)malloc(len * 2 + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[2*i] = hexdig[(data[i] >> 4) & 0xF];
        out[2*i + 1] = hexdig[data[i] & 0xF];
    }
    out[len*2] = '\0';
    return out;
}

// Returns a newly allocated message string; caller must free it.
char* register_user(sqlite3* db,
                    const char* username,
                    const char* password,
                    const char* first_name,
                    const char* last_name,
                    const char* email) {
    if (!username) username = "";
    if (!first_name) first_name = "";
    if (!last_name) last_name = "";
    if (!email) email = "";

    if (!is_valid_username(username)) return strdup("Invalid input: username");
    if (!is_valid_password(password)) return strdup("Invalid input: password");
    if (strlen(first_name) == 0 || strlen(first_name) > 100) return strdup("Invalid input: first name");
    if (strlen(last_name) == 0 || strlen(last_name) > 100) return strdup("Invalid input: last name");
    if (!is_valid_email(email)) return strdup("Invalid input: email");

    sqlite3_stmt* check = NULL;
    const char* check_sql = "SELECT 1 FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, check_sql, -1, &check, NULL) != SQLITE_OK) {
        if (check) sqlite3_finalize(check);
        return strdup("Internal error");
    }
    sqlite3_bind_text(check, 1, username, -1, SQLITE_TRANSIENT);
    int step = sqlite3_step(check);
    if (step == SQLITE_ROW) {
        sqlite3_finalize(check);
        return strdup("Username already exists");
    }
    sqlite3_finalize(check);

    unsigned char salt[SALT_LEN_C];
    if (RAND_bytes(salt, SALT_LEN_C) != 1) {
        return strdup("Internal error");
    }

    unsigned char dk[KEY_LEN_C];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt, SALT_LEN_C, PBKDF2_ITERATIONS_C, EVP_sha256(),
                          KEY_LEN_C, dk) != 1) {
        return strdup("Internal error");
    }

    char* hash_hex = hex_encode(dk, KEY_LEN_C);
    if (!hash_hex) return strdup("Internal error");

    const char* ins_sql = "INSERT INTO users (username, first_name, last_name, email, password_hash, salt, iterations) "
                          "VALUES (?,?,?,?,?,?,?);";
    sqlite3_stmt* ins = NULL;
    if (sqlite3_prepare_v2(db, ins_sql, -1, &ins, NULL) != SQLITE_OK) {
        free(hash_hex);
        if (ins) sqlite3_finalize(ins);
        return strdup("Internal error");
    }
    sqlite3_bind_text(ins, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 2, first_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 3, last_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 4, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 5, hash_hex, -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(ins, 6, salt, SALT_LEN_C, SQLITE_TRANSIENT);
    sqlite3_bind_int(ins, 7, PBKDF2_ITERATIONS_C);

    char* result = NULL;
    if (sqlite3_step(ins) == SQLITE_DONE) {
        result = strdup("Registration succeeded");
    } else {
        result = strdup("Internal error");
    }

    sqlite3_finalize(ins);
    free(hash_hex);
    return result;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("Internal error\n");
        return 0;
    }
    if (!init_db(db)) {
        printf("Internal error\n");
        sqlite3_close(db);
        return 0;
    }

    char* msg1 = register_user(db, "alice", "StrongPass#123", "Alice", "Anderson", "alice@example.com");
    printf("%s\n", msg1); free(msg1);
    char* msg2 = register_user(db, "alice", "StrongPass#123", "Alice", "Anderson", "alice@example.com");
    printf("%s\n", msg2); free(msg2);
    char* msg3 = register_user(db, "bob", "AnotherStrong#123", "Bob", "Baker", "bob@invalid");
    printf("%s\n", msg3); free(msg3);
    char* msg4 = register_user(db, "charlie", "short", "Charlie", "Clark", "charlie@example.com");
    printf("%s\n", msg4); free(msg4);
    char* msg5 = register_user(db, "dave", "Yetanother$Pass9", "Dave", "Doe", "dave@example.com");
    printf("%s\n", msg5); free(msg5);

    sqlite3_close(db);
    return 0;
}