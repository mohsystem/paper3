#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_STOCKS 50
#define STOCK_NAME_LEN 11 // 10 chars for name + 1 for null terminator

// Structure to represent a stock holding
typedef struct {
    char name[STOCK_NAME_LEN];
    int quantity;
} Stock;

// Simulating a database table with a fixed-size array of structs
Stock portfolio[MAX_STOCKS];
int stock_count = 0;

/**
 * @brief This function is called by buy_stock to perform a subsequent action.
 * In a real application, this could be logging, sending a notification, etc.
 * 
 * @param stockName The name of the stock that was purchased.
 */
void buy_function(const char* stockName) {
    printf("LOG: Post-purchase action for %s completed.\n", stockName);
}

/**
 * @brief Simulates buying a stock. It takes the stock name and quantity,
 * validates the input, "inserts" it into the simulated database,
 * and then calls a subsequent function.
 * 
 * @param stockName The ticker symbol of the stock to buy (e.g., "AAPL").
 * @param quantity The number of shares to buy.
 */
void buy_stock(const char* stockName, int quantity) {
    printf("\nAttempting to buy %d of %s...\n", quantity, stockName ? stockName : "NULL");

    // --- Security: Input Validation ---
    if (stockName == NULL || stockName[0] == '\0') {
        printf("Error: Stock name cannot be empty.\n");
        return;
    }
    if (quantity <= 0) {
        printf("Error: Quantity must be greater than zero.\n");
        return;
    }
    
    // Sanitize input: Create a mutable, uppercase, and size-limited copy
    char sanitized_name[STOCK_NAME_LEN];
    int i = 0;
    // Copy and convert to uppercase, ensuring we don't read past the source or write past the destination
    for (i = 0; i < STOCK_NAME_LEN - 1 && stockName[i] != '\0'; i++) {
        sanitized_name[i] = toupper((unsigned char)stockName[i]);
    }
    sanitized_name[i] = '\0'; // Null-terminate the string

    printf("Inserting %d of %s into the database.\n", quantity, sanitized_name);

    // --- "Database" Insertion ---
    // In a real C application with a DB like SQLite, you would use sqlite3_prepare_v2 and sqlite3_bind_*
    // to prevent SQL injection.

    // Simulating the database update:
    // First, check if the stock already exists in the portfolio
    int found_index = -1;
    for (i = 0; i < stock_count; i++) {
        if (strcmp(portfolio[i].name, sanitized_name) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) { // Stock exists, update quantity
        portfolio[found_index].quantity += quantity;
    } else { // New stock, add if there is space
        if (stock_count < MAX_STOCKS) {
            // Use strncpy for safe string copy to prevent buffer overflow
            strncpy(portfolio[stock_count].name, sanitized_name, STOCK_NAME_LEN - 1);
            portfolio[stock_count].name[STOCK_NAME_LEN - 1] = '\0'; // Ensure null termination
            portfolio[stock_count].quantity = quantity;
            stock_count++;
        } else {
            printf("Error: Database is full. Cannot add new stock.\n");
            return;
        }
    }
    
    printf("Successfully updated portfolio for %s.\n", sanitized_name);

    // Call the subsequent function as per the prompt
    buy_function(sanitized_name);
}

int main() {
    printf("--- Starting Stock Purchase Simulation ---\n");

    // Test Case 1: Buy a new stock
    buy_stock("GOOG", 10);

    // Test Case 2: Buy more of an existing stock
    buy_stock("goog", 5); // Using lowercase to test sanitization

    // Test Case 3: Buy another new stock
    buy_stock("MSFT", 20);
    
    // Test Case 4: Invalid quantity
    buy_stock("TSLA", 0);

    // Test Case 5: Invalid stock name (NULL)
    buy_stock(NULL, 100);

    printf("\n--- Final Portfolio Status ---\n");
    if (stock_count == 0) {
        printf("Portfolio is empty.\n");
    } else {
        for (int i = 0; i < stock_count; i++) {
            printf("Stock: %s, Quantity: %d\n", portfolio[i].name, portfolio[i].quantity);
        }
    }

    return 0;
}