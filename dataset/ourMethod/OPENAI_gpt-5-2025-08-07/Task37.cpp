#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sqlite3.h>

// Simple user record
struct User {
    int id = 0;
    std::string username;
    std::string email;
    std::string created_at;
};

// Validate username: [A-Za-z0-9_]{3,32}
bool isValidUsername(const std::string& s) {
    if (s.size() < 3 || s.size() > 32) return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
            return false;
        }
    }
    return true;
}

// ISO-8601 UTC timestamp (YYYY-MM-DDTHH:MM:SSZ)
std::string nowIso8601() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[32];
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm) == 0) {
        return "1970-01-01T00:00:00Z";
    }
    return std::string(buf);
}

bool initDb(sqlite3* db) {
    if (!db) return false;
    const char* createSql =
        "PRAGMA foreign_keys=ON;"
        "CREATE TABLE IF NOT EXISTS users ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL UNIQUE CHECK(length(username) BETWEEN 3 AND 32),"
        "  email TEXT NOT NULL CHECK(length(email) BETWEEN 5 AND 128),"
        "  created_at TEXT NOT NULL"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, createSql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool insertUser(sqlite3* db, const std::string& username, const std::string& email, const std::string& createdAt) {
    if (!db) return false;
    if (!isValidUsername(username)) return false;
    if (email.size() < 5 || email.size() > 128) return false;

    const char* sql = "INSERT INTO users(username, email, created_at) VALUES (?1, ?2, ?3);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        return false;
    }
    bool ok = false;
    do {
        if (sqlite3_bind_text(stmt, 1, username.c_str(), (int)username.size(), SQLITE_STATIC) != SQLITE_OK) break;
        if (sqlite3_bind_text(stmt, 2, email.c_str(), (int)email.size(), SQLITE_STATIC) != SQLITE_OK) break;
        if (sqlite3_bind_text(stmt, 3, createdAt.c_str(), (int)createdAt.size(), SQLITE_STATIC) != SQLITE_OK) break;
        if (sqlite3_step(stmt) != SQLITE_DONE) break;
        ok = true;
    } while (false);
    sqlite3_finalize(stmt);
    return ok;
}

// Retrieve a user by username using a prepared statement. Returns true if found.
bool getUserByUsername(sqlite3* db, const std::string& username, User& outUser) {
    if (!db) return false;
    if (!isValidUsername(username)) return false;

    const char* sql = "SELECT id, username, email, created_at FROM users WHERE username = ?1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        return false;
    }

    bool found = false;
    do {
        if (sqlite3_bind_text(stmt, 1, username.c_str(), (int)username.size(), SQLITE_STATIC) != SQLITE_OK) break;

        int stepRc = sqlite3_step(stmt);
        if (stepRc == SQLITE_ROW) {
            outUser.id = sqlite3_column_int(stmt, 0);
            const unsigned char* uname = sqlite3_column_text(stmt, 1);
            const unsigned char* email = sqlite3_column_text(stmt, 2);
            const unsigned char* created = sqlite3_column_text(stmt, 3);
            outUser.username = uname ? reinterpret_cast<const char*>(uname) : "";
            outUser.email = email ? reinterpret_cast<const char*>(email) : "";
            outUser.created_at = created ? reinterpret_cast<const char*>(created) : "";
            found = true;
        } else if (stepRc == SQLITE_DONE) {
            found = false;
        } else {
            found = false;
        }
    } while (false);

    sqlite3_finalize(stmt);
    return found;
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Failed to open SQLite database.\n";
        if (db) sqlite3_close(db);
        return 1;
    }

    if (!initDb(db)) {
        std::cerr << "Failed to initialize DB.\n";
        sqlite3_close(db);
        return 1;
    }

    const std::string ts = nowIso8601();
    // Seed sample users
    if (!insertUser(db, "alice", "alice@example.com", ts)) { std::cerr << "Insert alice failed.\n"; }
    if (!insertUser(db, "bob", "bob@example.com", ts)) { std::cerr << "Insert bob failed.\n"; }
    if (!insertUser(db, "charlie", "charlie@example.com", ts)) { std::cerr << "Insert charlie failed.\n"; }

    // 5 test cases (including invalid and injection attempt)
    std::vector<std::string> testUsernames = {
        "alice",
        "bob",
        "charlie",
        "doesnotexist",
        "bob' OR '1'='1" // should be rejected by validator
    };

    for (const auto& uname : testUsernames) {
        User u;
        bool ok = getUserByUsername(db, uname, u);
        std::cout << "Query username=\"" << uname << "\": ";
        if (ok) {
            std::cout << "FOUND -> id=" << u.id
                      << ", username=" << u.username
                      << ", email=" << u.email
                      << ", created_at=" << u.created_at << "\n";
        } else {
            std::cout << "NOT FOUND or INVALID INPUT\n";
        }
    }

    sqlite3_close(db);
    return 0;
}