#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>

// Securely checks if a username exists in the SQLite database at dbPath.
// Returns true if exists, false otherwise.
bool usernameExists(const std::string& dbPath, const std::string& username) {
    if (dbPath.empty() || username.empty()) return false;

    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return exists;
}

bool execSQL(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

int main() {
    const std::string dbPath = "users_cpp.db";
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cout << "DB init failed\n";
        if (db) sqlite3_close(db);
        return 0;
    }

    // Initialize DB
    bool ok = execSQL(db, "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL)");
    if (ok) ok = execSQL(db, "INSERT OR IGNORE INTO users (username) VALUES ('alice'), ('bob'), ('eve')");
    sqlite3_close(db);
    if (!ok) {
        std::cout << "DB init failed\n";
        return 0;
    }

    std::vector<std::string> tests = {
        "alice",
        "bob",
        "charlie",
        "",
        "Robert'); DROP TABLE users;--"
    };
    for (const auto& t : tests) {
        bool exists = usernameExists(dbPath, t);
        std::cout << "usernameExists(\"" << t << "\") = " << (exists ? "true" : "false") << "\n";
    }
    return 0;
}