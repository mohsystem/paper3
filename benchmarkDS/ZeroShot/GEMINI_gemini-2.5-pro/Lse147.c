#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// A simple struct to represent a stock order
typedef struct {
    char* stockName;
    int quantity;
} Order;

// In-memory dynamic array to act as a simple database
Order* database = NULL;
size_t db_count = 0;

// Helper function to check if a string is alphanumeric
int is_alphanumeric(const char* str) {
    if (str == NULL || *str == '\0') return 0; // Not valid if null or empty
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i])) {
            return 0; // False
        }
    }
    return 1; // True
}

// Helper function to check if a string is only whitespace
int is_whitespace(const char* str) {
    if (str == NULL) return 1;
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isspace((unsigned char)str[i])) {
            return 0; // False
        }
    }
    return 1; // True
}


/**
 * @brief Simulates buying a stock by validating input and adding it to an in-memory database.
 * This function demonstrates secure C practices like input validation and memory management.
 *
 * @param stockName The name of the stock (e.g., "AAPL"). Must be alphanumeric and not empty.
 * @param quantity The number of shares to buy. Must be a positive integer.
 * @return A const char* (string literal) indicating the result of the operation.
 */
const char* buy_order(const char* stockName, int quantity) {
    // 1. Security: Validate stock name - ensure it's not null, empty, or just whitespace.
    if (stockName == NULL || stockName[0] == '\0' || is_whitespace(stockName)) {
        return "Error: Stock name cannot be empty.";
    }

    // 2. Security: Sanitize stock name - ensure it contains only allowed characters.
    if (!is_alphanumeric(stockName)) {
        return "Error: Stock name must be alphanumeric.";
    }

    // 3. Security: Validate quantity - ensure it's a positive number.
    if (quantity <= 0) {
        return "Error: Quantity must be greater than zero.";
    }

    // 4. "Insert" into the database if all checks pass.
    // Resize the database array to hold one more element
    Order* temp = realloc(database, (db_count + 1) * sizeof(Order));
    if (temp == NULL) {
        // Security: Handle memory allocation failure gracefully
        return "Error: System out of memory.";
    }
    database = temp;

    // Allocate memory for the new stock name and copy it
    // Security: Use strdup (or equivalent) for safe memory allocation and copy
    database[db_count].stockName = malloc(strlen(stockName) + 1);
    if (database[db_count].stockName == NULL) {
        // In a real app, you might want to shrink the database allocation back
        return "Error: System out of memory for stock name.";
    }
    strcpy(database[db_count].stockName, stockName);
    
    database[db_count].quantity = quantity;
    db_count++;

    // 5. Simulate redirection. Note: Returning a non-literal string would require
    // careful memory management (e.g., static buffer). Here, we only return literals.
    return "SUCCESS: Order placed. Redirecting to stock_view.";
}

// Security: Function to free all allocated memory
void cleanup_database() {
    for (size_t i = 0; i < db_count; i++) {
        free(database[i].stockName); // Free each allocated string
    }
    free(database); // Free the array of structs
    database = NULL;
    db_count = 0;
}

int main() {
    // --- Test Cases ---
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Valid order
    printf("1. %s (Stock: %s, Qty: %d)\n", buy_order("AAPL", 100), "AAPL", 100);

    // Test Case 2: Invalid quantity (zero)
    printf("2. %s\n", buy_order("GOOG", 0));

    // Test Case 3: Invalid stock name (contains special characters)
    printf("3. %s\n", buy_order("MSFT-X", 50));

    // Test Case 4: Invalid stock name (empty)
    printf("4. %s\n", buy_order("  ", 200));
    
    // Test Case 5: Valid order after some failures
    printf("5. %s (Stock: %s, Qty: %d)\n", buy_order("TSLA", 25), "TSLA", 25);

    // --- Display Database Content ---
    printf("\n--- Current Database State ---\n");
    if (db_count == 0) {
        printf("Database is empty.\n");
    } else {
        for (size_t i = 0; i < db_count; ++i) {
            printf("Entry %zu: Stock=%s, Quantity=%d\n", i + 1, database[i].stockName, database[i].quantity);
        }
    }

    // Security: Clean up all dynamically allocated memory before exiting
    cleanup_database();
    printf("\nDatabase memory cleaned up.\n");

    return 0;
}