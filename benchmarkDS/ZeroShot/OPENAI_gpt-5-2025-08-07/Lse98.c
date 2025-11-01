#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define HASH_LEN 32
#define ITERATIONS 200000

int is_valid_username(const char* u) {
    size_t len;
    if (u == NULL) return 0;
    len = strlen(u);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!(isalnum(c) || c == '_' || c == '.' || c == '-')) return 0;
    }
    return 1;
}

int is_strong_password(const char* p) {
    size_t len;
    int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
    if (p == NULL) return 0;
    len = strlen(p);
    if (len < 8) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (islower(c)) hasLower = 1;
        else if (isupper(c)) hasUpper = 1;
        else if (isdigit(c)) hasDigit = 1;
        else hasSpecial = 1;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

void to_hex(const unsigned char* data, size_t len, char* out) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[2*i]     = hex[(data[i] >> 4) & 0xF];
        out[2*i + 1] = hex[data[i] & 0xF];
    }
    out[2*len] = '\0';
}

int register_user(sqlite3* db, const char* username, const char* password) {
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    char hexHash[HASH_LEN*2 + 1];
    char hexSalt[SALT_LEN*2 + 1];

    if (!db || !username || !password) return 0;
    if (!is_valid_username(username) || !is_strong_password(password)) return 0;

    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return 0;
    }

    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN,
                          ITERATIONS, EVP_sha256(), HASH_LEN, hash) != 1) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return 0;
    }

    to_hex(hash, HASH_LEN, hexHash);
    to_hex(salt, SALT_LEN, hexSalt);

    const char* sql = "INSERT INTO users(username, password_hash, salt, iterations) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        OPENSSL_cleanse(hash, sizeof(hash));
        OPENSSL_cleanse(salt, sizeof(salt));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hexHash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hexSalt, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, ITERATIONS);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    OPENSSL_cleanse(hash, sizeof(hash));
    OPENSSL_cleanse(salt, sizeof(salt));

    return rc == SQLITE_DONE ? 1 : 0;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("DB open failed\n");
        return 1;
    }
    const char* create_sql =
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL, "
        "iterations INTEGER NOT NULL);";
    char* err = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        printf("Create table failed: %s\n", err ? err : "");
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 1;
    }

    const char* users[5] = {"alice","bob","charlie","bad user","alice"};
    const char* pwds[5]  = {"Str0ng!Pwd1","weak","Anoth3r$trongPass","GoodPass1!","Another$tr0ng1"};
    for (int i = 0; i < 5; i++) {
        int ok = register_user(db, users[i], pwds[i]);
        printf("Register %s: %d\n", users[i], ok);
    }

    const char* q = "SELECT username, iterations FROM users ORDER BY id";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* u = sqlite3_column_text(stmt, 0);
            int it = sqlite3_column_int(stmt, 1);
            printf("DB -> %s iter=%d\n", u ? (const char*)u : "", it);
        }
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);
    return 0;
}