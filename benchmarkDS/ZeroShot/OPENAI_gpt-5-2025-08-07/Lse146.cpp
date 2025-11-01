#include <sqlite3.h>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

static const std::regex USERNAME_RE("^[A-Za-z0-9._-]{1,64}$");
static const std::regex STOCK_RE("^[A-Za-z0-9 ._-]{1,64}$");
static const char* DB_PATH = "lse146_orders_cpp.db";

static bool ensure_table_exists(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "stock_name TEXT NOT NULL,"
        "quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

static bool validate_inputs(const std::string& username, const std::string& stock_name, int quantity) {
    if (!std::regex_match(username, USERNAME_RE)) return false;
    if (!std::regex_match(stock_name, STOCK_RE)) return false;
    if (quantity < 1 || quantity > 1000000) return false;
    return true;
}

static std::string url_encode(const std::string& s) {
    std::ostringstream oss;
    for (unsigned char c : s) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            oss << c;
        } else {
            oss << '%' << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)c;
            oss << std::nouppercase << std::dec;
        }
    }
    return oss.str();
}

// Returns redirect URL on success; empty string on failure
std::string place_buy_order(const std::string& username, const std::string& stock_name, int stock_quantity) {
    if (!validate_inputs(username, stock_name, stock_quantity)) {
        return "";
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "";
    }

    if (!ensure_table_exists(db)) {
        sqlite3_close(db);
        return "";
    }

    const char* sql = "INSERT INTO buy_orders(username, stock_name, quantity) VALUES(?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return "";
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stock_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (rc != SQLITE_DONE) {
        return "";
    }

    return "/profile/" + url_encode(username);
}

int main() {
    // 5 test cases
    std::vector<std::tuple<std::string, std::string, int>> tests = {
        {"alice", "AAPL", 10},
        {"bob-01", "Microsoft Corp", 250},
        {"charlie", "GOOG", 0},          // invalid quantity
        {"dave", "Bad'Stock", 5},        // invalid stock name
        {"eve", "TSLA", 100}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        auto& t = tests[i];
        std::string res = place_buy_order(std::get<0>(t), std::get<1>(t), std::get<2>(t));
        if (res.empty()) {
            std::cout << "Test" << (i + 1) << ": INVALID\n";
        } else {
            std::cout << "Test" << (i + 1) << ": " << res << "\n";
        }
    }
    return 0;
}