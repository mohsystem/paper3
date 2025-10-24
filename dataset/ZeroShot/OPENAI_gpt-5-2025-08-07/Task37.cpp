#include <sqlite3.h>
#include <string>
#include <iostream>
#include <stdexcept>

// JSON string escaper that returns a quoted JSON string
static std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out.push_back('"');
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out.push_back(static_cast<char>(c));
                }
        }
    }
    out.push_back('"');
    return out;
}

// Securely query user info. Returns JSON string or "NOT_FOUND"
std::string get_user_info(sqlite3* db, const std::string& username) {
    if (!db || username.empty()) return "NOT_FOUND";
    sqlite3_busy_timeout(db, 5000);
    const char* sql = "SELECT username, full_name, email, created_at FROM users WHERE username = ?1";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return "NOT_FOUND";
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return "NOT_FOUND";
    }
    std::string result = "NOT_FOUND";
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::string uname  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string fname  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string ctime  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result = std::string("{") +
                 "\"username\":" + json_escape(uname) + "," +
                 "\"full_name\":" + json_escape(fname) + "," +
                 "\"email\":" + json_escape(email) + "," +
                 "\"created_at\":" + json_escape(ctime) +
                 "}";
    }
    sqlite3_finalize(stmt);
    return result;
}

static void exec_or_throw(sqlite3* db, const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string msg = err ? err : "Unknown error";
        sqlite3_free(err);
        throw std::runtime_error(msg);
    }
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Failed to open DB\n";
        return 1;
    }
    try {
        exec_or_throw(db, "PRAGMA foreign_keys = ON");
        exec_or_throw(db,
            "CREATE TABLE users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "full_name TEXT NOT NULL,"
            "email TEXT NOT NULL,"
            "created_at TEXT NOT NULL"
            ")"
        );

        // Seed data using prepared statement
        const char* ins = "INSERT INTO users (username, full_name, email, created_at) VALUES (?, ?, ?, ?)";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, ins, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("prepare insert failed");
        }
        struct Row { const char* u; const char* f; const char* e; const char* c; };
        Row rows[] = {
            {"alice", "Alice Anderson", "alice@example.com", "2023-01-01T00:00:00Z"},
            {"bob", "Bob Brown", "bob@example.com", "2023-02-02T00:00:00Z"},
            {"carol", "Carol Clark", "carol@example.com", "2023-03-03T00:00:00Z"},
            {"dave", "Dave Davis", "dave@example.com", "2023-04-04T00:00:00Z"},
        };
        for (auto& r : rows) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
            sqlite3_bind_text(stmt, 1, r.u, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, r.f, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, r.e, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, r.c, -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("insert failed");
            }
        }
        sqlite3_finalize(stmt);

        // 5 test cases
        std::string tests[] = {"alice", "bob", "doesnotexist", "carol", "alice' OR '1'='1"};
        for (const auto& u : tests) {
            std::string res = get_user_info(db, u);
            std::cout << "Query for '" << u << "': " << res << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        sqlite3_close(db);
        return 1;
    }
    sqlite3_close(db);
    return 0;
}