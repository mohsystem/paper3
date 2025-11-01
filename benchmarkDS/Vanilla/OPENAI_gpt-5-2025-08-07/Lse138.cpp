#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

// Requires SQLite3 development headers and library to compile/link.
// Example compile: g++ -std=c++17 app.cpp -lsqlite3 -o app
#include <sqlite3.h>

// Connect to DB (in-memory)
sqlite3* connect_db() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        return nullptr;
    }
    return db;
}

bool exec_sql(sqlite3* db, const std::string& sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

bool seed_db(sqlite3* db, const std::vector<std::pair<std::string,int>>& data) {
    if (!exec_sql(db, "CREATE TABLE users (user_id TEXT PRIMARY KEY, is_moderator INTEGER NOT NULL);")) return false;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "INSERT INTO users(user_id, is_moderator) VALUES(?, ?);", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    for (auto& row : data) {
        sqlite3_bind_text(stmt, 1, row.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, row.second);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);
    return true;
}

// Execute a query to check if the user is a moderator
bool is_moderator(sqlite3* db, const std::string& user_id) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT is_moderator FROM users WHERE user_id = ?;", -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    bool result = false;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int val = sqlite3_column_int(stmt, 0);
        result = (val != 0);
    } else {
        result = false;
    }
    sqlite3_finalize(stmt);
    return result;
}

// Function called when the route is invoked
std::string handle_route(sqlite3* db, const std::string& user_id) {
    return is_moderator(db, user_id) ? "true" : "false";
}

// Minimal "web app" router abstraction
struct Router {
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> routes;

    void add_route(const std::string& path, std::function<std::string(const std::string&)> handler) {
        routes[path] = std::move(handler);
    }

    std::string call(const std::string& path, const std::string& user_id) const {
        auto it = routes.find(path);
        if (it == routes.end()) return "404";
        return it->second(user_id);
    }
};

int main() {
    sqlite3* db = connect_db();
    if (!db) {
        std::cerr << "DB connection failed\n";
        return 1;
    }
    if (!seed_db(db, {{"1",1},{"2",0},{"3",1},{"4",0},{"5",1}})) {
        std::cerr << "DB seed failed\n";
        sqlite3_close(db);
        return 1;
    }

    Router app;
    // Define a route for the app
    app.add_route("/isModerator", [&](const std::string& user_id) {
        return handle_route(db, user_id);
    });

    // 5 test cases
    std::cout << app.call("/isModerator", "1") << "\n";   // true
    std::cout << app.call("/isModerator", "2") << "\n";   // false
    std::cout << app.call("/isModerator", "3") << "\n";   // true
    std::cout << app.call("/isModerator", "4") << "\n";   // false
    std::cout << app.call("/isModerator", "999") << "\n"; // false

    sqlite3_close(db);
    return 0;
}