#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sqlite3.h>

static std::string json_escape(const std::string& s) {
    std::ostringstream o;
    for (char c : s) {
        switch (c) {
            case '\\': o << "\\\\"; break;
            case '"':  o << "\\\""; break;
            case '\b': o << "\\b";  break;
            case '\f': o << "\\f";  break;
            case '\n': o << "\\n";  break;
            case '\r': o << "\\r";  break;
            case '\t': o << "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    o << "\\u" << std::hex << std::uppercase << (int)c;
                } else {
                    o << c;
                }
        }
    }
    return o.str();
}

int initDatabase(sqlite3* db) {
    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY,"
        "username TEXT UNIQUE NOT NULL,"
        "full_name TEXT NOT NULL,"
        "email TEXT NOT NULL"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return 1;
    }

    const char* insert_sql = "INSERT OR IGNORE INTO users (id, username, full_name, email) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 2;
    }

    struct Row { int id; const char* u; const char* f; const char* e; };
    Row rows[] = {
        {1, "alice", "Alice Wonderland", "alice@example.com"},
        {2, "bob", "Bob Builder", "bob@builder.com"},
        {3, "carol", "Carol Singer", "carol@songs.org"},
        {4, "dave", "Dave Grohl", "dave@foofighters.com"}
    };

    for (auto& r : rows) {
        sqlite3_reset(stmt);
        sqlite3_bind_int(stmt, 1, r.id);
        sqlite3_bind_text(stmt, 2, r.u, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, r.f, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, r.e, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return 3;
        }
    }
    sqlite3_finalize(stmt);
    return 0;
}

std::string getUserByUsername(sqlite3* db, const std::string& username) {
    const char* sql = "SELECT id, username, full_name, email FROM users WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::string();
    }
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    std::string result;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* u = sqlite3_column_text(stmt, 1);
        const unsigned char* f = sqlite3_column_text(stmt, 2);
        const unsigned char* e = sqlite3_column_text(stmt, 3);

        std::ostringstream o;
        o << "{\"id\":" << id
          << ",\"username\":\"" << json_escape(reinterpret_cast<const char*>(u)) << "\""
          << ",\"full_name\":\"" << json_escape(reinterpret_cast<const char*>(f)) << "\""
          << ",\"email\":\"" << json_escape(reinterpret_cast<const char*>(e)) << "\"}";
        result = o.str();
    }
    sqlite3_finalize(stmt);
    return result;
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Failed to open database\n";
        return 1;
    }
    if (initDatabase(db) != 0) {
        std::cerr << "Failed to initialize database\n";
        sqlite3_close(db);
        return 1;
    }

    std::vector<std::string> tests = {"alice", "bob", "carol", "dave", "nonexistent"};
    for (const auto& t : tests) {
        std::string res = getUserByUsername(db, t);
        std::cout << "Query for '" << t << "': " << (res.empty() ? "null" : res) << "\n";
    }

    sqlite3_close(db);
    return 0;
}