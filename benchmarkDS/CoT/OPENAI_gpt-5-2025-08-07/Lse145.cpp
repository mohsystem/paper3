#include <iostream>
#include <stdexcept>
#include <string>
#include <regex>
#include <algorithm>
#include <cctype>
#include <sqlite3.h>

// Step 1-5: Implement secure input validation and parameterized DB insertion.

class Lse145 {
private:
    static sqlite3* db;

    static void ensure_db() {
        if (db == nullptr) {
            if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
                throw std::runtime_error("Failed to open database.");
            }
            const char* ddl =
                "CREATE TABLE IF NOT EXISTS purchases ("
                "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  stock_name TEXT NOT NULL,"
                "  quantity INTEGER NOT NULL,"
                "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                ");";
            char* err = nullptr;
            if (sqlite3_exec(db, ddl, nullptr, nullptr, &err) != SQLITE_OK) {
                std::string msg = "DDL error: ";
                if (err) { msg += err; sqlite3_free(err); }
                throw std::runtime_error(msg);
            }
        }
    }

public:
    // buy_function: validate and normalize stock symbol
    static std::string buy_function(const std::string& stockName) {
        if (stockName.empty()) {
            throw std::invalid_argument("Stock name cannot be empty.");
        }
        // Trim spaces
        size_t start = stockName.find_first_not_of(" \t\r\n");
        size_t end = stockName.find_last_not_of(" \t\r\n");
        if (start == std::string::npos) {
            throw std::invalid_argument("Stock name cannot be empty.");
        }
        std::string s = stockName.substr(start, end - start + 1);
        static const std::regex re("^[A-Za-z]{1,10}$");
        if (!std::regex_match(s, re)) {
            throw std::invalid_argument("Stock name must be 1-10 alphabetic characters.");
        }
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::toupper(c); });
        return s;
    }

    // buy_stock: validate qty, call buy_function, insert with prepared statement, return row id
    static long long buy_stock(const std::string& stockName, int quantity) {
        if (quantity <= 0 || quantity > 1000000) {
            throw std::invalid_argument("Quantity must be between 1 and 1,000,000.");
        }
        ensure_db();
        std::string symbol = buy_function(stockName);

        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT INTO purchases (stock_name, quantity) VALUES (?, ?)";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement.");
        }

        int rc = SQLITE_OK;
        rc = sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_TRANSIENT);
        if (rc != SQLITE_OK) { sqlite3_finalize(stmt); throw std::runtime_error("Bind error (stock_name)."); }
        rc = sqlite3_bind_int(stmt, 2, quantity);
        if (rc != SQLITE_OK) { sqlite3_finalize(stmt); throw std::runtime_error("Bind error (quantity)."); }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Insert failed.");
        }
        sqlite3_finalize(stmt);
        return static_cast<long long>(sqlite3_last_insert_rowid(db));
    }
};

sqlite3* Lse145::db = nullptr;

static void runTest(const std::string& label, const std::string& name, int qty) {
    try {
        long long id = Lse145::buy_stock(name, qty);
        std::cout << label << " OK -> rowId=" << id << "\n";
    } catch (const std::exception& ex) {
        std::cout << label << " ERR -> " << ex.what() << "\n";
    }
}

int main() {
    // 5 test cases
    runTest("T1", "AAPL", 10);    // valid
    runTest("T2", "goog", 5);     // valid, normalized
    runTest("T3", "BAD!", 3);     // invalid name
    runTest("T4", "DROP", 0);     // invalid quantity
    runTest("T5", "msft", 100);   // valid
    return 0;
}