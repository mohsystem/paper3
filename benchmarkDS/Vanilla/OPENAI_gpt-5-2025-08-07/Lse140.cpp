#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>

bool userExists(sqlite3* db, const std::string& username) {
    if (db == nullptr) return false;
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return false;
    }

    int rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Cannot open database\n";
        return 1;
    }

    const char* create_sql = "CREATE TABLE users (username TEXT PRIMARY KEY);";
    const char* insert_sql =
        "INSERT INTO users(username) VALUES('alice');"
        "INSERT INTO users(username) VALUES('bob');"
        "INSERT INTO users(username) VALUES('carol');";

    char* err = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "Create table error: " << (err ? err : "") << "\n";
        sqlite3_free(err);
        sqlite3_close(db);
        return 1;
    }
    if (sqlite3_exec(db, insert_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "Insert error: " << (err ? err : "") << "\n";
        sqlite3_free(err);
        sqlite3_close(db);
        return 1;
    }

    std::vector<std::string> tests = {"alice", "bob", "dave", "", "Carol"}; // 5 test cases
    for (const auto& t : tests) {
        bool exists = userExists(db, t);
        std::cout << "userExists(\"" << t << "\") = " << (exists ? "true" : "false") << "\n";
    }

    sqlite3_close(db);
    return 0;
}