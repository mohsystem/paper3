#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>

#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

static const int PBKDF2_ITERATIONS_CPP = 100000;
static const int SALT_LEN_CPP = 16;
static const int KEY_LEN_CPP = 32;

bool isValidUsername(const std::string& u) {
    static const std::regex re("^[A-Za-z0-9_]{3,32}$");
    return std::regex_match(u, re);
}

bool isValidPassword(const std::string& p) {
    return p.size() >= 8 && p.size() <= 1024;
}

bool isValidEmail(const std::string& e) {
    static const std::regex re("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");
    return std::regex_match(e, re);
}

std::string toHex(const unsigned char* data, size_t len) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        oss << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return oss.str();
}

bool initDb(sqlite3* db) {
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
    char* err = nullptr;
    if (sqlite3_exec(db, sql1, nullptr, nullptr, &err) != SQLITE_OK) { sqlite3_free(err); return false; }
    if (sqlite3_exec(db, sql2, nullptr, nullptr, &err) != SQLITE_OK) { sqlite3_free(err); return false; }
    if (sqlite3_exec(db, sql3, nullptr, nullptr, &err) != SQLITE_OK) { sqlite3_free(err); return false; }
    return true;
}

std::string registerUser(sqlite3* db,
                         const std::string& username,
                         const std::string& password,
                         const std::string& firstName,
                         const std::string& lastName,
                         const std::string& email) {
    std::string u = username, f = firstName, l = lastName, e = email;
    auto trim = [](std::string& s){
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end = s.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) { s.clear(); return; }
        s = s.substr(start, end - start + 1);
    };
    trim(u); trim(f); trim(l); trim(e);

    if (!isValidUsername(u)) return "Invalid input: username";
    if (!isValidPassword(password)) return "Invalid input: password";
    if (f.empty() || f.size() > 100) return "Invalid input: first name";
    if (l.empty() || l.size() > 100) return "Invalid input: last name";
    if (!isValidEmail(e)) return "Invalid input: email";

    sqlite3_stmt* check = nullptr;
    const char* checkSql = "SELECT 1 FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, checkSql, -1, &check, nullptr) != SQLITE_OK) return "Internal error";
    sqlite3_bind_text(check, 1, u.c_str(), -1, SQLITE_TRANSIENT);
    int step = sqlite3_step(check);
    if (step == SQLITE_ROW) {
        sqlite3_finalize(check);
        return "Username already exists";
    }
    sqlite3_finalize(check);

    unsigned char salt[SALT_LEN_CPP];
    if (RAND_bytes(salt, SALT_LEN_CPP) != 1) {
        return "Internal error";
    }

    unsigned char dk[KEY_LEN_CPP];
    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                          salt, SALT_LEN_CPP, PBKDF2_ITERATIONS_CPP, EVP_sha256(),
                          KEY_LEN_CPP, dk) != 1) {
        return "Internal error";
    }

    const char* insSql = "INSERT INTO users (username, first_name, last_name, email, password_hash, salt, iterations) "
                         "VALUES (?,?,?,?,?,?,?);";
    sqlite3_stmt* ins = nullptr;
    if (sqlite3_prepare_v2(db, insSql, -1, &ins, nullptr) != SQLITE_OK) {
        return "Internal error";
    }
    std::string hashHex = toHex(dk, KEY_LEN_CPP);
    sqlite3_bind_text(ins, 1, u.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 2, f.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 3, l.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 4, e.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(ins, 5, hashHex.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(ins, 6, salt, SALT_LEN_CPP, SQLITE_TRANSIENT);
    sqlite3_bind_int(ins, 7, PBKDF2_ITERATIONS_CPP);

    if (sqlite3_step(ins) != SQLITE_DONE) {
        sqlite3_finalize(ins);
        return "Internal error";
    }
    sqlite3_finalize(ins);
    return "Registration succeeded";
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cout << "Internal error\n";
        return 0;
    }
    if (!initDb(db)) {
        std::cout << "Internal error\n";
        sqlite3_close(db);
        return 0;
    }

    std::cout << registerUser(db, "alice", "StrongPass#123", "Alice", "Anderson", "alice@example.com") << "\n";
    std::cout << registerUser(db, "alice", "StrongPass#123", "Alice", "Anderson", "alice@example.com") << "\n";
    std::cout << registerUser(db, "bob", "AnotherStrong#123", "Bob", "Baker", "bob@invalid") << "\n";
    std::cout << registerUser(db, "charlie", "short", "Charlie", "Clark", "charlie@example.com") << "\n";
    std::cout << registerUser(db, "dave", "Yetanother$Pass9", "Dave", "Doe", "dave@example.com") << "\n";

    sqlite3_close(db);
    return 0;
}