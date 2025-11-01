#include <iostream>
#include <string>
#include <cctype>
#include <sqlite3.h>

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static bool is_valid_stock_name(const std::string& input) {
    std::string name = trim(input);
    if (name.empty() || name.size() > 50) return false;
    for (unsigned char ch : name) {
        if (!(std::isalnum(ch) || ch == ' ' || ch == '.' || ch == '_' || ch == '-')) {
            return false;
        }
    }
    return true;
}

bool init_db(const std::string& db_path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "stock_name TEXT NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, ddl, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

// Simulates the buy_order route: validates input, inserts into DB securely, returns redirect path or error message.
std::string buy_order(const std::string& db_path, const std::string& stock_name_in, int quantity) {
    if (!is_valid_stock_name(stock_name_in)) {
        return "ERROR: Invalid stock name";
    }
    if (quantity <= 0 || quantity > 1000000) {
        return "ERROR: Invalid quantity";
    }
    std::string stock_name = trim(stock_name_in);

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "ERROR: Database failure";
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return "ERROR: Database failure";
    }

    const char* sql = "INSERT INTO buy_orders (stock_name, quantity) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return "ERROR: Database failure";
    }

    sqlite3_bind_text(stmt, 1, stock_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return "ERROR: Database failure";
    }

    if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return "ERROR: Database failure";
    }

    sqlite3_close(db);
    return "/stock_view";
}

int main() {
    std::string dbp = "cpp_buy_orders.db";
    std::cout << "Init DB: " << (init_db(dbp) ? "OK" : "FAIL") << std::endl;

    // 5 test cases
    std::cout << buy_order(dbp, "AAPL", 10) << std::endl;                        // expected redirect
    std::cout << buy_order(dbp, "MSFT-2025", 1) << std::endl;                    // expected redirect
    std::cout << buy_order(dbp, "AAPL; DROP TABLE buy_orders;", 5) << std::endl; // expected error
    std::cout << buy_order(dbp, "GOOG", 0) << std::endl;                         // expected error
    std::cout << buy_order(dbp, std::string(51, 'A'), 2) << std::endl;           // expected error
    return 0;
}