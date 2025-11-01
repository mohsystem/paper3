
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sqlite3.h>

/* Constants for validation */
#define MAX_STOCK_NAME_LENGTH 50
#define MAX_QUANTITY 1000000
#define MIN_QUANTITY 1
#define MAX_ERROR_MSG_LENGTH 256

/* Secure string copy with bounds checking */
void secure_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return;
    }
    
    /* Use strncpy and ensure null termination */
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\\0';
}

/* Validate stock name - prevents injection attacks */
int is_valid_stock_name(const char* stock_name) {
    size_t len;
    size_t i;
    
    /* Check for NULL pointer */
    if (stock_name == NULL) {
        return 0;
    }
    
    /* Check length with bounds check */
    len = strnlen(stock_name, MAX_STOCK_NAME_LENGTH + 1);
    if (len == 0 || len > MAX_STOCK_NAME_LENGTH) {
        return 0;
    }
    
    /* Validate characters - only alphanumeric, space, hyphen, underscore */
    for (i = 0; i < len; i++) {
        char c = stock_name[i];
        if (!isalnum((unsigned char)c) && c != ' ' && c != '-' && c != '_') {
            return 0;
        }
    }
    
    return 1;
}

/* Validate quantity - prevents overflow */
int is_valid_quantity(int quantity) {
    return quantity >= MIN_QUANTITY && quantity <= MAX_QUANTITY;
}

/* Trim whitespace from string */
void trim_string(char* str) {
    char* end;
    size_t len;
    
    if (str == NULL) {
        return;
    }
    
    /* Trim leading space */
    while (isspace((unsigned char)*str)) {
        str++;
    }
    
    if (*str == '\\0') {
        return;
    }
    
    /* Trim trailing space */
    len = strnlen(str, MAX_STOCK_NAME_LENGTH);
    end = str + len - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }
    
    /* Null terminate */
    *(end + 1) = '\\0';
}

/* Get secure database connection */
sqlite3* get_secure_connection(void) {
    sqlite3* db = NULL;
    const char* db_path;
    int rc;
    
    /* Get database path from environment - never hardcode credentials */
    db_path = getenv("DB_PATH");
    if (db_path == NULL) {
        db_path = "stocks.db";
    }
    
    /* Validate path - prevent path traversal */
    if (strstr(db_path, "..") != NULL || db_path[0] == '/') {
        fprintf(stderr, "Invalid database path\\n");
        return NULL;
    }
    
    /* Open database with error checking */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return NULL;
    }
    
    /* Enable foreign keys */
    sqlite3_exec(db, "PRAGMA foreign_keys = ON", NULL, NULL, NULL);
    
    return db;
}

/* Buy function - processes stock name */
int buy_function(const char* stock_name, char* result, size_t result_size) {
    char sanitized[MAX_STOCK_NAME_LENGTH + 1] = {0}; /* Initialize to zero */
    
    /* Input validation - treat all input as untrusted */
    if (!is_valid_stock_name(stock_name)) {
        secure_strcpy(result, result_size, "Invalid stock name");
        return 0;
    }
    
    /* Sanitize input - bounds checked copy */
    secure_strcpy(sanitized, sizeof(sanitized), stock_name);
    trim_string(sanitized);
    
    /* Build result string with bounds checking */
    snprintf(result, result_size, "Processing purchase for stock: %s", sanitized);
    
    return 1;
}

/* Main stock purchase function */
int buy_stock(const char* stock_name, int quantity, char* result, size_t result_size) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    char sanitized_name[MAX_STOCK_NAME_LENGTH + 1] = {0};
    char buy_result[MAX_ERROR_MSG_LENGTH] = {0};
    char timestamp[64] = {0};
    time_t now;
    struct tm* timeinfo;
    int rc;
    int success = 0;
    
    /* Initialize result buffer */
    if (result != NULL && result_size > 0) {
        result[0] = '\\0';
    }
    
    /* Input validation - fail early */
    if (!is_valid_stock_name(stock_name)) {
        secure_strcpy(result, result_size, "Error: Invalid stock name format");
        return 0;
    }
    
    if (!is_valid_quantity(quantity)) {
        snprintf(result, result_size, 
                "Error: Invalid quantity. Must be between %d and %d", 
                MIN_QUANTITY, MAX_QUANTITY);
        return 0;
    }
    
    /* Sanitize inputs - bounds checked */
    secure_strcpy(sanitized_name, sizeof(sanitized_name), stock_name);
    trim_string(sanitized_name);
    
    /* Call buy_function */
    if (!buy_function(sanitized_name, buy_result, sizeof(buy_result))) {
        secure_strcpy(result, result_size, "Error: Unable to process stock");
        return 0;
    }
    
    /* Get secure database connection */
    db = get_secure_connection();
    if (db == NULL) {
        secure_strcpy(result, result_size, "Error: Unable to establish database connection");
        return 0;
    }
    
    /* Use parameterized query to prevent SQL injection */
    rc = sqlite3_prepare_v2(db, 
        "INSERT INTO stock_purchases (stock_name, quantity, purchase_date) VALUES (?, ?, ?)",
        -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement\\n");
        sqlite3_close(db);
        secure_strcpy(result, result_size, "Error: Unable to complete purchase");
        return 0;
    }
    
    /* Bind parameters - type-safe binding */
    sqlite3_bind_text(stmt, 1, sanitized_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    /* Get current timestamp */
    now = time(NULL);
    timeinfo = localtime(&now);
    if (timeinfo != NULL) {
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
        sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_TRANSIENT);
    }
    
    /* Execute statement */
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        snprintf(result, result_size, "Stock purchase successful: %s", buy_result);
        success = 1;
    } else {
        secure_strcpy(result, result_size, "Stock purchase failed");
    }
    
    /* Clean up resources - prevent leaks */
    if (stmt != NULL) {
        sqlite3_finalize(stmt);
    }
    if (db != NULL) {
        sqlite3_close(db);
    }
    
    return success;
}

/* Test cases */
int main(void) {
    char result[MAX_ERROR_MSG_LENGTH] = {0};
    
    printf("Test Case 1 - Valid stock purchase:\\n");
    buy_stock("AAPL", 10, result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 2 - Valid stock with hyphen:\\n");
    buy_stock("TECH-CORP", 50, result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 3 - Invalid stock name (SQL injection attempt):\\n");
    buy_stock("AAPL'; DROP TABLE stock_purchases; --", 10, result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 4 - Invalid quantity (negative):\\n");
    buy_stock("GOOGL", -5, result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nTest Case 5 - Invalid quantity (too large):\\n");
    buy_stock("MSFT", 2000000, result, sizeof(result));
    printf("%s\\n", result);
    
    return 0;
}
