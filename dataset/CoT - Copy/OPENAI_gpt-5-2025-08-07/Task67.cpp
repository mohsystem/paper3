// Chain-of-Through Process in Code (C++):
// 1) Problem understanding: Fetch customer by 'customerusername' from 'customer' table; username from command-line; return structured data.
// 2) Security requirements: Use prepared statements, validate input, manage resources safely.
// 3) Secure coding generation: Implement regex validation, sqlite3_prepare_v2 with bound parameter, robust error checks.
// 4) Code review: Ensure no SQL injection, finalize statements, close DB, handle errors carefully.
// 5) Secure code output: Final secure C++ implementation below.

#include <sqlite3.h>
#include <iostream>
#include <string>
#include <vector>
#include <regex>

struct Customer {
    int id;
    std::string username;
    std::string fullname;
    std::string email;
    std::string created_at;
};

static bool is_valid_username(const std::string& username) {
    static const std::regex re("^[A-Za-z0-9_.-]{1,64}$");
    return std::regex_match(username, re);
}

static int ensure_schema_and_seed(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS customer ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "customerusername TEXT UNIQUE NOT NULL,"
        "fullname TEXT NOT NULL,"
        "email TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")";
    char* err = nullptr;
    int rc = sqlite3_exec(db, create_sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return rc;
    }

    const char* upsert_sql =
        "INSERT INTO customer (customerusername, fullname, email) "
        "VALUES (?, ?, ?) "
        "ON CONFLICT(customerusername) DO UPDATE SET "
        "fullname=excluded.fullname, email=excluded.email";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, upsert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return rc;

    auto bind_exec = [&](const char* u, const char* f, const char* e)->int {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, u, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, f, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, e, -1, SQLITE_TRANSIENT);
        int r = sqlite3_step(stmt);
        return (r == SQLITE_DONE) ? SQLITE_OK : r;
    };

    rc = bind_exec("alice", "Alice Anderson", "alice@example.com"); if (rc != SQLITE_OK) { sqlite3_finalize(stmt); return rc; }
    rc = bind_exec("bob", "Bob Brown", "bob@example.com"); if (rc != SQLITE_OK) { sqlite3_finalize(stmt); return rc; }
    rc = bind_exec("charlie", "Charlie Clark", "charlie@example.com"); if (rc != SQLITE_OK) { sqlite3_finalize(stmt); return rc; }
    rc = bind_exec("dora", "Dora Dawson", "dora@example.net"); if (rc != SQLITE_OK) { sqlite3_finalize(stmt); return rc; }
    rc = bind_exec("eve", "Eve Edwards", "eve@example.org"); if (rc != SQLITE_OK) { sqlite3_finalize(stmt); return rc; }

    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

static bool get_customer_by_username(sqlite3* db, const std::string& username, Customer& out) {
    if (!is_valid_username(username)) {
        return false;
    }
    const char* sql =
        "SELECT id, customerusername, fullname, email, COALESCE(created_at,'') "
        "FROM customer WHERE customerusername = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    bool found = false;
    if (rc == SQLITE_ROW) {
        out.id = sqlite3_column_int(stmt, 0);
        out.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out.fullname = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        out.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        out.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

int main(int argc, char** argv) {
    sqlite3* db = nullptr;
    if (sqlite3_open("customers.db", &db) != SQLITE_OK) {
        std::cout << "{\"error\":\"database_error\"}\n";
        if (db) sqlite3_close(db);
        return 0;
    }

    if (ensure_schema_and_seed(db) != SQLITE_OK) {
        std::cout << "{\"error\":\"database_error\"}\n";
        sqlite3_close(db);
        return 0;
    }

    std::vector<std::string> tests;
    if (argc >= 2 && argv[1] && std::string(argv[1]).size() > 0) {
        tests.push_back(argv[1]);
    }
    std::vector<std::string> defaults = {"alice", "bob", "charlie", "not_exist_user_123", "eve"};
    for (const auto& d : defaults) {
        if (tests.size() >= 5) break;
        bool exists = false;
        for (const auto& t : tests) if (t == d) { exists = true; break; }
        if (!exists) tests.push_back(d);
    }
    while (tests.size() < 5) tests.push_back("dora");
    if (tests.size() > 5) tests.resize(5);

    for (const auto& username : tests) {
        Customer c;
        if (get_customer_by_username(db, username, c)) {
            std::cout << "{"
                      << "\"id\":" << c.id << ","
                      << "\"customerusername\":\"" << c.username << "\","
                      << "\"fullname\":\"" << c.fullname << "\","
                      << "\"email\":\"" << c.email << "\","
                      << "\"created_at\":\"" << c.created_at << "\""
                      << "}\n";
        } else {
            std::cout << "{\"customerusername\":\"" << username << "\",\"result\":\"not_found_or_invalid\"}\n";
        }
    }

    sqlite3_close(db);
    return 0;
}