#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "sqlite3.h"

static bool isValidUsername(const std::string& username) {
    static const std::regex re("^[A-Za-z0-9_.-]{1,64}$");
    return std::regex_match(username, re);
}

static bool isValidStockName(const std::string& stock) {
    static const std::regex re("^[A-Za-z0-9 _.-]{1,128}$");
    return std::regex_match(stock, re);
}

static std::string iso8601NowUTC() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

static std::string urlEncode(const std::string& s) {
    static const char hex[] = "0123456789ABCDEF";
    std::string out;
    for (unsigned char c : s) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
    }
    return out;
}

std::string placeBuyOrder(const std::string& username, const std::string& stock_name, int stock_quantity) {
    if (!isValidUsername(username)) throw std::invalid_argument("Invalid username");
    if (!isValidStockName(stock_name)) throw std::invalid_argument("Invalid stock name");
    if (stock_quantity <= 0 || stock_quantity > 1000000) throw std::invalid_argument("Invalid stock quantity");

    sqlite3* db = nullptr;
    if (sqlite3_open("orders_cpp.db", &db) != SQLITE_OK) {
        throw std::runtime_error("DB open failed");
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "stock_name TEXT NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "created_at TEXT NOT NULL"
        ");";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "create failed";
        sqlite3_free(errMsg);
        sqlite3_close(db);
        throw std::runtime_error(err);
    }

    const char* insert_sql = "INSERT INTO buy_orders (username, stock_name, quantity, created_at) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error(err);
    }

    std::string ts = iso8601NowUTC();
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stock_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);
    sqlite3_bind_text(stmt, 4, ts.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw std::runtime_error(err);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return std::string("/profile/") + urlEncode(username);
}

int main() {
    struct Test { std::string u, s; int q; };
    Test tests[5] = {
        {"alice", "AAPL", 10},
        {"bob.smith", "GOOGL", 25},
        {"charlie-d", "MSFT", 1000},
        {"diana", "Tesla Inc", 15},
        {"eve_99", "AMZN", 5}
    };
    for (const auto& t : tests) {
        try {
            std::cout << "Redirect to: " << placeBuyOrder(t.u, t.s, t.q) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}