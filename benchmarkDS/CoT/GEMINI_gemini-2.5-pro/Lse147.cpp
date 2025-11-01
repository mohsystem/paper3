#include <iostream>
#include <string>
#include <vector>
#include "sqlite3.h"

/**
 * In a real web application, this function would be part of a route handler
 * (e.g., using a library like Crow or Pistache). The stockName and quantity
 * would be parsed from an HTTP request. The handler would then issue a
 * redirect response.
 *
 * This simulation focuses on the secure database insertion logic using the
 * SQLite C-API, which is common in C++ applications.
 *
 * @param db The SQLite database handle.
 * @param stockName The name of the stock to buy.
 * @param quantity The quantity of the stock to buy.
 * @return True on success, false on failure.
 */
bool buyOrder(sqlite3* db, const std::string& stockName, int quantity) {
    sqlite3_stmt* stmt = nullptr;
    // SQL query with placeholders (?) to prevent SQL injection.
    const char* sql = "INSERT INTO orders(stock_name, quantity) VALUES(?, ?);";

    // Prepare the SQL statement. This compiles the SQL and checks for syntax errors.
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind values to the placeholders. This is the key step for security.
    // The library handles any special characters safely.
    // Index 1 is the first '?'
    sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, quantity);

    // Execute the statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt); // Clean up statement
        return false;
    }

    std::cout << "Successfully inserted order for " << quantity << " of " << stockName << std::endl;
    
    // Clean up the statement object to free resources.
    sqlite3_finalize(stmt);
    return true;
}

// Callback function for SELECT queries.
static int viewCallback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << ", ";
    }
    std::cout << std::endl;
    return 0;
}

void viewOrders(sqlite3* db) {
    const char* sql = "SELECT * FROM orders;";
    char* zErrMsg = 0;
    
    std::cout << "\n--- Current Orders in Database ---" << std::endl;
    int rc = sqlite3_exec(db, sql, viewCallback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    std::cout << "--------------------------------\n" << std::endl;
}


int main() {
    sqlite3* db;
    // Using a file-based database. Can be ":memory:" for in-memory.
    int rc = sqlite3_open("stocks_cpp.db", &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    // Create the table.
    const char* createTableSql = "CREATE TABLE IF NOT EXISTS orders ("
                                 " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 " stock_name TEXT NOT NULL,"
                                 " quantity INTEGER NOT NULL,"
                                 " order_date DATETIME DEFAULT CURRENT_TIMESTAMP"
                                 ");";
    char* zErrMsg = 0;
    rc = sqlite3_exec(db, createTableSql, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    } else {
        std::cout << "Table 'orders' is ready." << std::endl;
    }

    // --- 5 Test Cases ---
    std::cout << "\n--- Running 5 Test Cases ---" << std::endl;
    buyOrder(db, "AAPL", 10);
    buyOrder(db, "GOOGL", 5);
    buyOrder(db, "MSFT", 15);
    buyOrder(db, "AMZN", 2);
    // Test case with a potentially malicious name to show safety
    buyOrder(db, "TSLA'); DROP TABLE orders; --", 1);
    std::cout << "--- Test Cases Finished ---\n" << std::endl;

    // View all orders to verify insertions.
    viewOrders(db);

    // Close the database connection.
    sqlite3_close(db);
    std::cout << "Database connection closed." << std::endl;
    return 0;
}