#include <stdio.h>
#include <string.h>
#include <stdbool.h> // For bool type
#include <ctype.h>   // For isspace

// Forward declaration
void buy_function(const char* stockName);

/**
 * @brief Helper function to check if a string is NULL, empty, or only contains whitespace.
 * 
 * @param str The string to check.
 * @return true if the string is invalid, false otherwise.
 */
bool is_string_invalid(const char* str) {
    if (str == NULL) {
        return true;
    }
    while (*str) {
        if (!isspace((unsigned char)*str)) {
            return false; // Found a non-whitespace character
        }
        str++;
    }
    return true; // String is all whitespace or empty
}

/**
 * @brief Simulates buying a stock and securely inserting the transaction into a database.
 * 
 * This function demonstrates input validation and the use of parameterized queries
 * to prevent SQL injection vulnerabilities.
 * 
 * @param stockName The name of the stock to buy (e.g., "AAPL").
 * @param quantity The number of shares to buy.
 */
void buy_stock(const char* stockName, int quantity) {
    // --- Security: Input Validation ---
    // 1. Ensure stockName is not NULL and not empty/whitespace.
    // 2. Ensure quantity is a positive number.
    if (is_string_invalid(stockName)) {
        printf("Error: Stock name cannot be null or empty.\n\n");
        return;
    }
    if (quantity <= 0) {
        printf("Error: Quantity must be a positive number.\n\n");
        return;
    }

    printf("Processing purchase of %d shares of %s\n", quantity, stockName);

    // --- Security: Using Parameterized Queries (Prepared Statements) ---
    // This is the standard way to prevent SQL injection. C database libraries
    // (like libpq for PostgreSQL or ODBC drivers) provide functions for this.

    // An insecure and dangerous way (vulnerable to SQL injection and buffer overflows):
    // char query[256];
    // sprintf(query, "INSERT INTO stocks (name, quantity) VALUES ('%s', %d)", stockName, quantity);
    
    // The secure, parameterized approach (simulated):
    printf("Simulating secure database insertion...\n");
    printf("  Prepared Statement: INSERT INTO stocks (name, quantity) VALUES (?, ?)\n");
    printf("  Binding parameter 1 (string): %s\n", stockName);
    printf("  Binding parameter 2 (int): %d\n", quantity);
    printf("  Execution successful.\n");

    // Call the other function as required.
    buy_function(stockName);
    printf("\n"); // Add a newline for cleaner test output.
}

/**
 * @brief Simulates some secondary logic that runs after a stock purchase is recorded.
 * @param stockName The name of the stock.
 */
void buy_function(const char* stockName) {
    printf("-> buy_function: Executing secondary logic for stock: %s\n", stockName);
}

int main() {
    printf("--- Running C Test Cases ---\n\n");

    // Test Case 1: Valid input
    buy_stock("AAPL", 10);

    // Test Case 2: Another valid input
    buy_stock("GOOG", 5);

    // Test Case 3: Invalid quantity (-5)
    buy_stock("MSFT", -5);

    // Test Case 4: Invalid stock name (NULL pointer)
    buy_stock(NULL, 100);

    // Test Case 5: Invalid stock name (whitespace)
    buy_stock("   ", 20);

    return 0;
}