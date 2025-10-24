// Chain-of-Through secure solution in C++ (Single-file).
// 1) Problem: Connect to SQLite, query user by username, return via output parameter.
// 2) Security: Prepared statements, no SQL concatenation, check errors, finalize statements.
// 3) Implementation below using sqlite3 C API with safe patterns.
// 4) Reviewed for resource cleanup and error handling.
// 5) Final secure code.

#include <sqlite3.h>
#include <iostream>
#include <string>

struct User {
    std::string username;
    std::string full_name;
    std::string email;
    std::string role;
};

// Returns 1 if found and fills out, 0 if not found, -1 on error
int get_user_info(sqlite3* db, const std::string& username, User* out) {
    if (!db) return -1;
    if (username.empty()) return 0;

    const char* sql = "SELECT username, full_name, email, role FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        if (out) {
            out->username   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            out->full_name  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            out->email      = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            out->role       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        }
        sqlite3_finalize(stmt);
        return 1;
    } else if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return 0;
    } else {
        sqlite3_finalize(stmt);
        return -1;
    }
}

int seed_test_data(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "  username TEXT PRIMARY KEY,"
        "  full_name TEXT NOT NULL,"
        "  email TEXT NOT NULL,"
        "  role TEXT NOT NULL"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        return -1;
    }

    const char* insert_sql = "INSERT INTO users (username, full_name, email, role) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        return -1;
    }

    auto insert_one = [&](const char* u, const char* f, const char* e, const char* r)->int {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        if (sqlite3_bind_text(stmt, 1, u, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        if (sqlite3_bind_text(stmt, 2, f, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        if (sqlite3_bind_text(stmt, 3, e, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        if (sqlite3_bind_text(stmt, 4, r, -1, SQLITE_TRANSIENT) != SQLITE_OK) return -1;
        int rc2 = sqlite3_step(stmt);
        if (rc2 != SQLITE_DONE) return -1;
        return 0;
    };

    int ok = 0;
    ok |= insert_one("alice", "Alice Johnson", "alice@example.com", "user");
    ok |= insert_one("bob", "Bob Smith", "bob@example.com", "user");
    ok |= insert_one("charlie", "Charlie Rose", "charlie@example.com", "manager");
    ok |= insert_one("admin", "System Admin", "admin@example.com", "admin");
    ok |= insert_one("eve", "Eve Adams", "eve@example.com", "auditor");

    sqlite3_finalize(stmt);
    return ok == 0 ? 0 : -1;
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cout << "Database error occurred.\n";
        if (db) sqlite3_close(db);
        return 0;
    }

    if (seed_test_data(db) != 0) {
        std::cout << "Database error occurred.\n";
        sqlite3_close(db);
        return 0;
    }

    const char* tests[5] = {
        "alice",
        "charlie",
        "nonexistent",
        "admin' OR '1'='1",
        "admin"
    };

    for (const char* uname : tests) {
        User u;
        int res = get_user_info(db, uname, &u);
        if (res == 1) {
            std::cout << "Query username: " << uname << " -> "
                      << "User{username='" << u.username
                      << "', fullName='" << u.full_name
                      << "', email='" << u.email
                      << "', role='" << u.role << "'}\n";
        } else if (res == 0) {
            std::cout << "Query username: " << uname << " -> Not found\n";
        } else {
            std::cout << "Error querying for username: " << uname << "\n";
        }
    }

    sqlite3_close(db);
    return 0;
}