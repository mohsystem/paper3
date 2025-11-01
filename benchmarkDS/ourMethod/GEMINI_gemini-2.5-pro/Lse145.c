#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>

#define MAX_STOCKS 100
#define MAX_STOCK_NAME_LEN 17 // 16 chars + null terminator

// Simulate a database table
typedef struct {
    char name[MAX_STOCK_NAME_LEN];
    int quantity;
} Stock;

static Stock stock_database[MAX_STOCKS];
static int stock_count = 0;

/**
 * @brief A secondary function called by buy_stock.
 * @param stock_name The name of the stock being processed.
 */
void buy_function(const char* stock_name) {
    if (stock_name == NULL) {
        // This should not happen if called from a sanitized context, but is good practice
        return; 
    }
    printf("[buy_function] INFO: Secondary processing for stock: %s\n", stock_name);
}

/**
 * @brief Validates if a character is valid for a stock name.
 * @param c The character to check.
 * @return true if valid, false otherwise.
 */
bool is_valid_stock_char(char c) {
    return isalnum((unsigned char)c) || c == '.' || c == '-';
}

/**
 * @brief Simulates buying a stock by validating input and adding it to an in-memory database.
 * @param stock_name The name/ticker of the stock to buy.
 * @param quantity The amount of stock to buy.
 * @return true if the purchase was successful, false otherwise.
 */
bool buy_stock(const char* stock_name, int quantity) {
    // Rule #1: Ensure all input is validated and sanitized
    // 1. Check for NULL pointer
    if (stock_name == NULL) {
        fprintf(stderr, "ERROR: Stock name cannot be null.\n");
        return false;
    }

    // 2. Check length constraints using strnlen for safety
    size_t len = strnlen(stock_name, MAX_STOCK_NAME_LEN);
    if (len == 0) {
        fprintf(stderr, "ERROR: Stock name cannot be empty.\n");
        return false;
    }
    if (len >= MAX_STOCK_NAME_LEN) {
        fprintf(stderr, "ERROR: Stock name exceeds maximum length of %d characters.\n", MAX_STOCK_NAME_LEN - 1);
        return false;
    }

    // 3. Validate format/characters
    for (size_t i = 0; i < len; ++i) {
        if (!is_valid_stock_char(stock_name[i])) {
            fprintf(stderr, "ERROR: Stock name contains invalid characters.\n");
            return false;
        }
    }

    // 4. Validate quantity
    if (quantity <= 0) {
        fprintf(stderr, "ERROR: Quantity must be a positive number.\n");
        return false;
    }
    
    // --- Processing ---
    printf("SUCCESS: Validation passed for %d shares of %s.\n", quantity, stock_name);
    
    // Simulate inserting/updating the record in the database
    int existing_stock_index = -1;
    for (int i = 0; i < stock_count; ++i) {
        if (strncmp(stock_database[i].name, stock_name, MAX_STOCK_NAME_LEN) == 0) {
            existing_stock_index = i;
            break;
        }
    }

    if (existing_stock_index != -1) { // Stock exists, update quantity
        // Check for potential integer overflow before adding
        if (stock_database[existing_stock_index].quantity > INT_MAX - quantity) {
             fprintf(stderr, "ERROR: Quantity integer overflow detected.\n");
             return false;
        }
        stock_database[existing_stock_index].quantity += quantity;
        printf("DATABASE: Updated portfolio. Now holding %d shares of %s.\n", 
               stock_database[existing_stock_index].quantity, stock_database[existing_stock_index].name);
    } else { // New stock, add to database if not full
        if (stock_count >= MAX_STOCKS) {
            fprintf(stderr, "ERROR: Database is full. Cannot add new stock.\n");
            return false;
        }
        // Rule #3: Ensure buffer operations adhere to boundaries
        strncpy(stock_database[stock_count].name, stock_name, MAX_STOCK_NAME_LEN - 1);
        stock_database[stock_count].name[MAX_STOCK_NAME_LEN - 1] = '\0'; // Ensure null termination
        stock_database[stock_count].quantity = quantity;
        
        printf("DATABASE: Added to portfolio. Now holding %d shares of %s.\n", 
            stock_database[stock_count].quantity, stock_database[stock_count].name);
        stock_count++;
    }
    
    // Call the secondary function
    buy_function(stock_name);

    return true;
}

int main() {
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Valid purchase
    printf("\n--- Test Case 1: Valid Input ---\n");
    buy_stock("IBM", 50);

    // Test Case 2: Invalid stock name (NULL)
    printf("\n--- Test Case 2: Null Stock Name ---\n");
    buy_stock(NULL, 10);

    // Test Case 3: Invalid stock name (too long)
    printf("\n--- Test Case 3: Stock Name Too Long ---\n");
    buy_stock("THISISAVERYVERYLONGSTOCKNAME", 5);

    // Test Case 4: Invalid stock name (invalid characters)
    printf("\n--- Test Case 4: Invalid Characters ---\n");
    buy_stock("BRK.A!", 1);

    // Test Case 5: Invalid quantity (zero)
    printf("\n--- Test Case 5: Invalid Quantity ---\n");
    buy_stock("F", 0);

    printf("\n--- Final Database State ---\n");
    for (int i = 0; i < stock_count; ++i) {
        printf("%s: %d\n", stock_database[i].name, stock_database[i].quantity);
    }

    return 0;
}