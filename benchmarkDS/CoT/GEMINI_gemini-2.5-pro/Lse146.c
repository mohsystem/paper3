#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Helper function to check if a string is null, empty, or only whitespace.
int isStringInvalid(const char* str) {
    if (str == NULL) {
        return 1; // True, string is invalid
    }
    const char *p = str;
    // Move pointer past any leading whitespace
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    // If we are at the end of the string, it was empty or all whitespace
    return *p == '\0';
}

/**
 * @brief Simulates placing a stock buy order with secure practices.
 * 
 * @param username The username of the person placing the order.
 * @param stockName The name of the stock to buy.
 * @param stockQuantity The quantity of the stock to buy.
 */
void placeBuyOrder(const char* username, const char* stockName, int stockQuantity) {
    // --- Input Validation ---
    // 1. Ensure username and stockName are not null or effectively empty.
    if (isStringInvalid(username)) {
        printf("ERROR: Username cannot be empty.\n");
        return;
    }
    if (isStringInvalid(stockName)) {
        printf("ERROR: Stock name cannot be empty.\n");
        return;
    }
    // 2. Ensure quantity is a positive number.
    if (stockQuantity <= 0) {
        printf("ERROR: Stock quantity must be a positive number.\n");
        return;
    }
    
    printf("Processing order for user: %s\n", username);
    
    // --- Simulate Secure Database Insertion ---
    // Using parameterized queries/prepared statements is crucial to prevent SQL Injection.
    // We simulate this by showing the query template and the bound parameters separately.
    printf("DATABASE: Preparing statement: INSERT INTO buy_orders (username, stock_name, quantity) VALUES (?, ?, ?);\n");
    printf("DATABASE: Binding parameter 1 (username) -> %s\n", username);
    printf("DATABASE: Binding parameter 2 (stock_name) -> %s\n", stockName);
    printf("DATABASE: Binding parameter 3 (quantity) -> %d\n", stockQuantity);
    printf("DATABASE: Execution successful. Order for %d of %s inserted.\n", stockQuantity, stockName);

    // --- Simulate Redirection ---
    printf("SYSTEM: Redirecting user '%s' to profile page.\n", username);
    printf("----------------------------------------\n");
}


int main() {
    // Test Case 1: Valid order
    placeBuyOrder("j_doe", "AAPL", 10);

    // Test Case 2: Another valid order
    placeBuyOrder("a_smith", "GOOGL", 5);

    // Test Case 3: Invalid order with zero quantity
    placeBuyOrder("b_jones", "TSLA", 0);

    // Test Case 4: Invalid order with empty stock name
    placeBuyOrder("c_white", "", 100);
    
    // Test Case 5: Invalid order with NULL username
    placeBuyOrder(NULL, "MSFT", 50);

    return 0;
}