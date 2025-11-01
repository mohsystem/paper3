
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>

#define MAX_QUANTITY 1000000
#define MIN_QUANTITY 1
#define MAX_USERNAME_LEN 50
#define MIN_USERNAME_LEN 3
#define MAX_STOCK_NAME_LEN 10

/* Validate that string contains only alphanumeric and allowed characters */
int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        fprintf(stderr, "Invalid username: NULL\\n");
        return 0;
    }
    
    len = strlen(username);
    
    /* Check length bounds */
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) {
        fprintf(stderr, "Username length must be between %d and %d\\n", 
                MIN_USERNAME_LEN, MAX_USERNAME_LEN);
        return 0;
    }
    
    /* Check character validity */
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            fprintf(stderr, "Username contains invalid characters\\n");
            return 0;
        }
    }
    
    return 1;
}

/* Validate stock name format */
int validate_stock_name(const char* stock_name) {
    size_t len;
    size_t i;
    
    if (stock_name == NULL) {
        fprintf(stderr, "Invalid stock name: NULL\\n");
        return 0;
    }
    
    len = strlen(stock_name);
    
    /* Check length */
    if (len == 0 || len > MAX_STOCK_NAME_LEN) {
        fprintf(stderr, "Stock name length invalid\\n");
        return 0;
    }
    
    /* Check character validity: alphanumeric, dot, underscore, hyphen */
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)stock_name[i]) && 
            stock_name[i] != '.' && 
            stock_name[i] != '_' && 
            stock_name[i] != '-') {
            fprintf(stderr, "Stock name contains invalid characters\\n");
            return 0;
        }
    }
    
    return 1;
}

/* Validate quantity range */
int validate_quantity(int quantity) {
    if (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY) {
        fprintf(stderr, "Quantity must be between %d and %d\\n", 
                MIN_QUANTITY, MAX_QUANTITY);
        return 0;
    }
    return 1;
}

/* Securely insert buy order into database */
int insert_buy_order(const char* username, const char* stock_name, int stock_quantity) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    const char* db_path;
    const char* sql;
    int rc;
    int result = 0;
    
    /* Validate all inputs */
    if (!validate_username(username)) {
        return 0;
    }
    
    if (!validate_stock_name(stock_name)) {
        return 0;
    }
    
    if (!validate_quantity(stock_quantity)) {
        return 0;
    }
    
    /* Get database path from environment variable */
    db_path = getenv("DB_PATH");
    if (db_path == NULL) {
        db_path = "orders.db";
    }
    
    /* Validate database path to prevent directory traversal */
    if (strstr(db_path, "..") != NULL || db_path[0] == '/') {
        fprintf(stderr, "Invalid database path\\n");
        return 0;
    }
    
    /* Open database connection */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database connection error\\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return 0;
    }
    
    /* Prepare parameterized query to prevent SQL injection */
    sql = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) "
          "VALUES (?, ?, ?, datetime('now'))";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Statement preparation error\\n");
        sqlite3_close(db);
        return 0;
    }
    
    /* Bind validated parameters */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);
    
    /* Execute the statement */
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        printf("Order inserted successfully\\n");
        result = 1;
    } else {
        fprintf(stderr, "Failed to insert order\\n");
        result = 0;
    }
    
    /* Clean up resources */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main(void) {
    /* Test case 1: Valid input */
    printf("Test 1: %d\\n", insert_buy_order("john_doe", "AAPL", 100));
    
    /* Test case 2: Valid input with different stock */
    printf("Test 2: %d\\n", insert_buy_order("jane_smith", "GOOGL", 50));
    
    /* Test case 3: Invalid stock name (SQL injection attempt) */
    printf("Test 3: %d\\n", insert_buy_order("user123", "AAPL'; DROP TABLE--", 10));
    
    /* Test case 4: Invalid quantity (negative) */
    printf("Test 4: %d\\n", insert_buy_order("alice_w", "MSFT", -5));
    
    /* Test case 5: Invalid quantity (exceeds maximum) */
    printf("Test 5: %d\\n", insert_buy_order("bob_jones", "TSLA", 2000000));
    
    return 0;
}
