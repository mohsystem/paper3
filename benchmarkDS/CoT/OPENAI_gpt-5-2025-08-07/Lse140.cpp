#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>

sqlite3* connect(const std::string& dbPath) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Failed to connect to database: " << sqlite3_errmsg(db) << std::endl;
        if (db) sqlite3_close(db);
        return nullptr;
    }
    return db;
}

bool checkUsernameExists(sqlite3* db, const std::string& username) {
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

void setupDatabase(sqlite3* db) {
    const char* createSql = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY)";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createSql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Create table failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    const char* insertSql = "INSERT OR IGNORE INTO users (username) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr) == SQLITE_OK) {
        std::vector<std::string> users = {"alice", "bob", "charlie"};
        for (const auto& u : users) {
            sqlite3_bind_text(stmt, 1, u.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
        }
        sqlite3_finalize(stmt);
    } else {
        std::cerr << "Prepare insert failed: " << sqlite3_errmsg(db) << std::endl;
    }
}

int main() {
    sqlite3* db = connect(":memory:");
    if (!db) return 1;

    setupDatabase(db);

    std::vector<std::string> tests = {"alice", "bob", "charlie", "dave", "eve"}; // 5 test cases
    for (const auto& user : tests) {
        bool exists = checkUsernameExists(db, user);
        std::cout << user << " -> " << (exists ? "true" : "false") << std::endl;
    }

    sqlite3_close(db);
    return 0;
}