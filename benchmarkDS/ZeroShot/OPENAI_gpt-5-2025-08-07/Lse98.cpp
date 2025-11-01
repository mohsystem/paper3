#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const int SALT_LEN = 16;
static const int HASH_LEN = 32;
static const int ITERATIONS = 200000;

bool is_valid_username(const std::string& u) {
    if (u.size() < 3 || u.size() > 32) return false;
    for (unsigned char c : u) {
        if (!(std::isalnum(c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}

bool is_strong_password(const std::string& p) {
    if (p.size() < 8) return false;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (unsigned char c : p) {
        if (std::islower(c)) hasLower = true;
        else if (std::isupper(c)) hasUpper = true;
        else if (std::isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    return hasLower && hasUpper && hasDigit && hasSpecial;
}

std::string to_hex(const unsigned char* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out[2*i]     = hex[(data[i] >> 4) & 0xF];
        out[2*i + 1] = hex[data[i] & 0xF];
    }
    return out;
}

bool secure_random_bytes(unsigned char* buf, int len) {
    if (RAND_bytes(buf, len) == 1) return true;
    return false;
}

bool register_user(sqlite3* db, const std::string& username, const std::string& password) {
    if (!db) return false;
    if (!is_valid_username(username) || !is_strong_password(password)) return false;

    unsigned char salt[SALT_LEN];
    if (!secure_random_bytes(salt, SALT_LEN)) {
        return false;
    }

    unsigned char hash[HASH_LEN];
    int ok = PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                               salt, SALT_LEN, ITERATIONS, EVP_sha256(),
                               HASH_LEN, hash);
    if (ok != 1) {
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }

    std::string hexHash = to_hex(hash, HASH_LEN);
    std::string hexSalt = to_hex(salt, SALT_LEN);

    const char* sql = "INSERT INTO users(username, password_hash, salt, iterations) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        OPENSSL_cleanse(hash, sizeof(hash));
        OPENSSL_cleanse(salt, sizeof(salt));
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hexHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, hexSalt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, ITERATIONS);

    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    sqlite3_finalize(stmt);

    OPENSSL_cleanse(hash, sizeof(hash));
    OPENSSL_cleanse(salt, sizeof(salt));

    return success;
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "DB open failed\n";
        return 1;
    }
    const char* create_sql =
        "CREATE TABLE users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL, "
        "iterations INTEGER NOT NULL);";
    char* err = nullptr, *errmsg = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "Create table failed: " << (err ? err : "") << "\n";
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 1;
    }
    std::vector<std::pair<std::string,std::string>> tests = {
        {"alice", "Str0ng!Pwd1"},
        {"bob", "weak"},
        {"charlie", "Anoth3r$trongPass"},
        {"bad user", "GoodPass1!"},
        {"alice", "Another$tr0ng1"},
    };
    for (auto& t : tests) {
        bool ok = register_user(db, t.first, t.second);
        std::cout << "Register " << t.first << ": " << (ok ? "1" : "0") << "\n";
    }
    const char* q = "SELECT username, iterations FROM users ORDER BY id";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* u = sqlite3_column_text(stmt, 0);
            int it = sqlite3_column_int(stmt, 1);
            std::cout << "DB -> " << (u ? reinterpret_cast<const char*>(u) : "") << " iter=" << it << "\n";
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}