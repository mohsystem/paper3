#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Constants for validation
#define MAX_USERNAME_LEN 50
#define MIN_USERNAME_LEN 3
#define MAX_STOCK_NAME_LEN 10
#define MIN_STOCK_NAME_LEN 1

// Forward declarations
void insertBuyOrderIntoDB(const char* username, const char* stockName, int quantity);
void redirectToProfile(const char* username);
bool validate_username(const char* username);
bool validate_stock_name(const char* stock_name);
bool processBuyOrder(const char* username, const char* stockName, int stockQuantity);

/**
 * Validates the username format.
 * - Must be between MIN and MAX length.
 * - Must contain only alphanumeric characters.
 */
bool validate_username(const char* username) {
    if (username == NULL) {
        return false;
    }
    size_t len = strlen(username);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum((unsigned char)username[i])) {
            return false;
        }
    }
    return true;
}

/**
 * Validates the stock name format.
 * - Must be between MIN and MAX length.
 * - Must contain only uppercase alphabetic characters.
 */
bool validate_stock_name(const char* stock_name) {
    if (stock_name == NULL) {
        return false;
    }
    size_t len = strlen(stock_name);
    if (len < MIN_STOCK_NAME_LEN || len > MAX_STOCK_NAME_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isupper((unsigned char)stock_name[i])) {
            return false;
        }
    }
    return true;
}

/**
 * Simulates inserting a buy order into a database using a secure, parameterized query pattern.
 */
void insertBuyOrderIntoDB(const char* username, const char* stockName, int quantity) {
    // This is a simulation of a secure database insertion.
    // In a real application, you would use a library like libpq or ODBC
    // with prepared statements to prevent SQL injection.
    printf("DATABASE: Preparing to execute INSERT INTO buy_orders (username, stock_name, quantity) VALUES (?, ?, ?)\n");
    printf("DATABASE: Binding parameter 1 (username) -> %s\n", username);
    printf("DATABASE: Binding parameter 2 (stock_name) -> %s\n", stockName);
    printf("DATABASE: Binding parameter 3 (quantity) -> %d\n", quantity);
    printf("DATABASE: Order for %d of %s by %s inserted successfully.\n", quantity, stockName, username);
}

/**
 * Simulates redirecting the user to their profile page.
 */
void redirectToProfile(const char* username) {
    printf("REDIRECT: Now redirecting to /profile/%s\n", username);
}

/**
 * Processes a stock buy order after validating the inputs.
 * @return true if the order was processed successfully, false otherwise.
 */
bool processBuyOrder(const char* username, const char* stockName, int stockQuantity) {
    // Rule #1: Input Validation
    if (!validate_username(username)) {
        fprintf(stderr, "Error: Invalid username format or length.\n");
        return false;
    }

    if (!validate_stock_name(stockName)) {
        fprintf(stderr, "Error: Invalid stock name format. Must be 1-%d uppercase letters.\n", MAX_STOCK_NAME_LEN);
        return false;
    }

    if (stockQuantity <= 0) {
        fprintf(stderr, "Error: Stock quantity must be a positive number.\n");
        return false;
    }

    printf("Input validation passed.\n");

    // Simulate database insertion
    insertBuyOrderIntoDB(username, stockName, stockQuantity);

    // Simulate redirection
    redirectToProfile(username);
    
    return true;
}

int main() {
    printf("--- Test Case 1: Valid Order ---\n");
    processBuyOrder("testuser123", "GOOGL", 10);
    printf("\n");

    printf("--- Test Case 2: Invalid Username (NULL) ---\n");
    processBuyOrder(NULL, "AAPL", 5);
    printf("\n");

    printf("--- Test Case 3: Invalid Stock Name (too long) ---\n");
    processBuyOrder("anotheruser", "VERYLONGSTOCKNAME", 20);
    printf("\n");

    printf("--- Test Case 4: Invalid Quantity (negative) ---\n");
    processBuyOrder("validuser", "TSLA", -100);
    printf("\n");

    printf("--- Test Case 5: Valid Order 2 ---\n");
    processBuyOrder("traderc", "IBM", 75);
    printf("\n");

    return 0;
}