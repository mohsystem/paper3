#include <sqlite3.h>
#include <string>
#include <sstream>
#include <iostream>

bool init_db(const std::string& db_path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS orders ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  stock TEXT NOT NULL,"
        "  qty INTEGER NOT NULL,"
        "  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

long long buy_order(const std::string& db_path, const std::string& stock, int qty) {
    if (stock.empty()) throw std::invalid_argument("stock is required");
    if (qty <= 0) throw std::invalid_argument("qty must be positive");

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return -1;
    }

    const char* sql = "INSERT INTO orders(stock, qty) VALUES(?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, stock.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, qty);

    long long rowid = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        rowid = sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rowid;
}

std::string stock_view(const std::string& db_path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return "";
    }

    const char* sql = "SELECT id, stock, qty, created_at FROM orders ORDER BY id";
    sqlite3_stmt* stmt = nullptr;
    std::ostringstream out;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            long long id = sqlite3_column_int64(stmt, 0);
            const unsigned char* stock = sqlite3_column_text(stmt, 1);
            int qty = sqlite3_column_int(stmt, 2);
            const unsigned char* created_at = sqlite3_column_text(stmt, 3);
            out << id << "," << (stock ? reinterpret_cast<const char*>(stock) : "")
                << "," << qty << "," << (created_at ? reinterpret_cast<const char*>(created_at) : "")
                << "\n";
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return out.str();
}

int main() {
    std::string db = "stocks_cpp.db";
    if (!init_db(db)) return 1;

    buy_order(db, "AAPL", 10);
    std::cout << stock_view(db);

    buy_order(db, "GOOG", 5);
    std::cout << stock_view(db);

    buy_order(db, "MSFT", 20);
    std::cout << stock_view(db);

    buy_order(db, "TSLA", 3);
    std::cout << stock_view(db);

    buy_order(db, "AMZN", 7);
    std::cout << stock_view(db);

    return 0;
}