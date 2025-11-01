#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define ITERATIONS 310000
#define KEY_LEN 32

static int init_schema(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash BLOB NOT NULL,"
        "salt BLOB NOT NULL,"
        "iterations INTEGER NOT NULL"
        ");";
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return 0;
    }
    return 1;
}

static int is_valid_username(const char* u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 64) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

int insert_user(sqlite3* db, const char* username, const char* password) {
    if (db == NULL || username == NULL || password == NULL) return 0;
    if (!is_valid_username(username)) return 0;
    if (strlen(password) < 8) return 0;

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return 0;
    }

    unsigned char key[KEY_LEN];
    if (!PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, ITERATIONS, EVP_sha256(), KEY_LEN, key)) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return 0;
    }

    const char* sql = "INSERT INTO users(username, password_hash, salt, iterations) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        OPENSSL_cleanse(key, sizeof(key));
        OPENSSL_cleanse(salt, sizeof(salt));
        return 0;
    }

    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    rc |= sqlite3_bind_blob(stmt, 2, key, KEY_LEN, SQLITE_TRANSIENT);
    rc |= sqlite3_bind_blob(stmt, 3, salt, SALT_LEN, SQLITE_TRANSIENT);
    rc |= sqlite3_bind_int(stmt, 4, ITERATIONS);

    int result = 0;
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        result = (rc == SQLITE_DONE) ? 1 : 0;
    }

    sqlite3_finalize(stmt);
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(salt, sizeof(salt));
    return result;
}

int main(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        printf("false\nfalse\nfalse\nfalse\nfalse\n");
        return 0;
    }
    if (!init_schema(db)) {
        printf("false\nfalse\nfalse\nfalse\nfalse\n");
        sqlite3_close(db);
        return 0;
    }

    printf("%s\n", insert_user(db, "alice", "CorrectHorseBatteryStaple1!") ? "true" : "false");
    printf("%s\n", insert_user(db, "bob", "S3cureP@ssw0rd!") ? "true" : "false");
    printf("%s\n", insert_user(db, "charlie", "anotherStrong#Pass123") ? "true" : "false");
    printf("%s\n", insert_user(db, "alice", "duplicateUser!") ? "true" : "false"); // duplicate
    printf("%s\n", insert_user(db, "bad name", "weak") ? "true" : "false");        // invalid

    sqlite3_close(db);
    return 0;
}