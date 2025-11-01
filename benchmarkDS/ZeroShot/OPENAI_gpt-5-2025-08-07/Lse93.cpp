#include <cstdio>
#include <cstring>
#include <string>
#include <regex>
#include <vector>
#include <iostream>

#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const std::regex USERNAME_RE("^[A-Za-z0-9_.-]{3,64}$");
static const int SALT_LEN = 16;
static const int ITERATIONS = 310000;
static const int KEY_LEN = 32; // 256-bit

static bool init_schema(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password_hash BLOB NOT NULL,"
        "salt BLOB NOT NULL,"
        "iterations INTEGER NOT NULL"
        ");";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

static bool is_valid_username(const std::string& u) {
    return std::regex_match(u, USERNAME_RE);
}

static bool derive_key(const std::string& password, const unsigned char* salt, int iterations, unsigned char* outKey, int outLen) {
    if (!PKCS5_PBKDF2_HMAC(password.c_str(), (int)password.size(), salt, SALT_LEN, iterations, EVP_sha256(), outLen, outKey)) {
        return false;
    }
    return true;
}

bool insert_user(sqlite3* db, const std::string& username, const std::string& password) {
    if (!db) return false;
    if (!is_valid_username(username)) return false;
    if (password.size() < 8) return false;

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        return false;
    }

    unsigned char key[KEY_LEN];
    bool ok = derive_key(password, salt, ITERATIONS, key, KEY_LEN);
    if (!ok) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }

    const char* sql = "INSERT INTO users(username, password_hash, salt, iterations) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        OPENSSL_cleanse(key, sizeof(key));
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }

    rc = sqlite3_bind_text(stmt, 1, username.c_str(), (int)username.size(), SQLITE_TRANSIENT);
    rc |= sqlite3_bind_blob(stmt, 2, key, KEY_LEN, SQLITE_TRANSIENT);
    rc |= sqlite3_bind_blob(stmt, 3, salt, SALT_LEN, SQLITE_TRANSIENT);
    rc |= sqlite3_bind_int(stmt, 4, ITERATIONS);

    bool result = false;
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        result = (rc == SQLITE_DONE);
    }

    sqlite3_finalize(stmt);
    OPENSSL_cleanse(key, sizeof(key));
    OPENSSL_cleanse(salt, sizeof(salt));
    return result;
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cout << "false\nfalse\nfalse\nfalse\nfalse\n";
        return 0;
    }
    if (!init_schema(db)) {
        std::cout << "false\nfalse\nfalse\nfalse\nfalse\n";
        sqlite3_close(db);
        return 0;
    }

    std::cout << (insert_user(db, "alice", "CorrectHorseBatteryStaple1!") ? "true" : "false") << "\n";
    std::cout << (insert_user(db, "bob", "S3cureP@ssw0rd!") ? "true" : "false") << "\n";
    std::cout << (insert_user(db, "charlie", "anotherStrong#Pass123") ? "true" : "false") << "\n";
    std::cout << (insert_user(db, "alice", "duplicateUser!") ? "true" : "false") << "\n"; // duplicate
    std::cout << (insert_user(db, "bad name", "weak") ? "true" : "false") << "\n";         // invalid

    sqlite3_close(db);
    return 0;
}