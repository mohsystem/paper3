// Chain-of-Through process in code generation for C++:
// 1. Problem understanding: Provide a function buy_order that inserts (stock_name, quantity) into a DB and then shows stock_view.
// 2. Security requirements: Validate input, use parameterized queries (sqlite3 prepared statements), handle resources.
// 3. Secure coding generation: Prepared statements, input validation with allowed characters, bounds checking.
// 4. Code review: Ensure sqlite3 objects finalized/closed, check return codes.
// 5. Secure code output: Final implementation with 5 test cases.
//
// Compile with: g++ -std=c++17 this_file.cpp -lsqlite3

#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <sqlite3.h>

static bool is_valid_stock_name(const std::string& name) {
    if (name.empty() || name.size() > 64) return false;
    for (unsigned char c : name) {
        if (!(std::isalnum(c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}

static int init_db(const std::string& path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 1;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " stock_name TEXT NOT NULL,"
        " quantity INTEGER NOT NULL CHECK(quantity > 0),"
        " created_at TEXT DEFAULT (datetime('now'))"
        ");";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 2;
    }
    sqlite3_close(db);
    return 0;
}

static std::string stock_view(const std::string& path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "";
    }
    const char* sql = "SELECT id, stock_name, quantity, created_at FROM buy_orders ORDER BY id ASC";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return "";
    }
    std::string out;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        int qty = sqlite3_column_int(stmt, 2);
        const unsigned char* created = sqlite3_column_text(stmt, 3);
        out += std::to_string(id) + "|" + (name ? reinterpret_cast<const char*>(name) : "") + "|" +
               std::to_string(qty) + "|" + (created ? reinterpret_cast<const char*>(created) : "") + "\n";
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return out;
}

static std::string buy_order(const std::string& path, const std::string& stock_name, int quantity) {
    if (!is_valid_stock_name(stock_name)) {
        throw std::invalid_argument("Invalid stock name");
    }
    if (quantity <= 0 || quantity > 1000000) {
        throw std::invalid_argument("Invalid quantity");
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        throw std::runtime_error("DB open failed");
    }
    const char* sql = "INSERT INTO buy_orders (stock_name, quantity) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Prepare failed");
    }
    sqlite3_bind_text(stmt, 1, stock_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw std::runtime_error("Insert failed");
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return stock_view(path);
}

int main() {
    std::string db = "cpp_demo.db";
    if (init_db(db) != 0) {
        std::cerr << "DB init failed\n";
        return 1;
    }

    try {
        std::cout << "After AAPL:\n" << buy_order(db, "AAPL", 10);
        std::cout << "After GOOGL:\n" << buy_order(db, "GOOGL", 5);
        std::cout << "After MSFT:\n" << buy_order(db, "MSFT", 15);
        std::cout << "After TSLA:\n" << buy_order(db, "TSLA", 1);
        std::cout << "After NVDA:\n" << buy_order(db, "NVDA", 20);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 2;
    }
    return 0;
}