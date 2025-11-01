#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

/**
 * In a real web application, this function would be called by a route handler
 * (e.g., using a library like libmicrohttpd). The stockName and quantity
 * would be parsed from an HTTP request. The handler would then issue a
 * redirect response.
 *
 * This simulation focuses on the secure database insertion logic using the
 * SQLite C-API.
 *
 * @param db The SQLite database handle.
 * @param stockName The name of the stock to buy.
 * @param quantity The quantity of the stock to buy.
 * @return 0 on success, 1 on failure.
 */
int buyOrder(sqlite3* db, const char* stockName, int quantity) {
    sqlite3_stmt* stmt = NULL;
    // SQL query with placeholders (?) to prevent SQL injection.
    const char* sql = "INSERT INTO orders(stock_name, quantity) VALUES(?, ?);";

    // Prepare the SQL statement. This compiles the SQL and checks for syntax errors.
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Bind values to the placeholders. This is the key step for security.
    // The library handles any special characters safely.
    // Index 1 is the first '?'
    sqlite3_bind_text(stmt, 1, stockName, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, quantity);

    // Execute the statement.
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt); // Clean up statement
        return 1;
    }

    printf("Successfully inserted order for %d of %s\n", quantity, stockName);
    
    // Clean up the statement object to free resources.
    sqlite3_finalize(stmt);
    return 0;
}

// Callback function for SELECT queries.
static int viewCallback(void* data, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s: %s, ", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void viewOrders(sqlite3* db) {
    const char* sql = "SELECT * FROM orders;";
    char* zErrMsg = 0;
    
    printf("\n--- Current Orders in Database ---\n");
    int rc = sqlite3_exec(db, sql, viewCallback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    printf("--------------------------------\n\n");
}


int main(int argc, char* argv[]) {
    sqlite3* db;
    // Using a file-based database. Can be ":memory:" for in-memory.
    int rc = sqlite3_open("stocks_c.db", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    } else {
        fprintf(stdout, "Opened database successfully\n");
    }

    // Create the table.
    const char* createTableSql = "CREATE TABLE IF NOT EXISTS orders ("
                                 " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 " stock_name TEXT NOT NULL,"
                                 " quantity INTEGER NOT NULL,"
                                 " order_date DATETIME DEFAULT CURRENT_TIMESTAMP"
                                 ");";
    char* zErrMsg = 0;
    rc = sqlite3_exec(db, createTableSql, NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Table 'orders' is ready.\n");
    }

    // --- 5 Test Cases ---
    printf("\n--- Running 5 Test Cases ---\n");
    buyOrder(db, "AAPL", 10);
    buyOrder(db, "GOOGL", 5);
    buyOrder(db, "MSFT", 15);
    buyOrder(db, "AMZN", 2);
    // Test case with a potentially malicious name to show safety
    buyOrder(db, "TSLA'); DROP TABLE orders; --", 1);
    printf("--- Test Cases Finished ---\n\n");

    // View all orders to verify insertions.
    viewOrders(db);

    // Close the database connection.
    sqlite3_close(db);
    printf("Database connection closed.\n");
    return 0;
}