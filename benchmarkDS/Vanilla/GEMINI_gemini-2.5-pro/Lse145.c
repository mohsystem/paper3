#include <stdio.h>
#include <string.h>

#define MAX_STOCKS 100
#define MAX_NAME_LEN 16

// Struct to represent a stock holding
typedef struct {
    char name[MAX_NAME_LEN];
    int quantity;
} StockHolding;

// Simulating a database table for stock holdings
StockHolding portfolio[MAX_STOCKS];
int portfolio_size = 0;

/**
 * This function is called by buy_stock to perform additional processing.
 * It takes the stock name as an argument.
 * @param stock_name The name of the stock.
 */
void buy_function(const char* stock_name) {
    printf("-> buy_function: Performing additional checks/logging for %s...\n", stock_name);
}

/**
 * Simulates buying a stock and inserting it into a database.
 * This function takes the stock name and quantity and updates the portfolio.
 * It then calls the buy_function.
 * @param stock_name The name of the stock to buy.
 * @param quantity The quantity of the stock to buy.
 */
void buy_stock(const char* stock_name, int quantity) {
    printf("Attempting to buy %d shares of %s.\n", quantity, stock_name);
    
    // Simulate inserting/updating the database
    int found_index = -1;
    for (int i = 0; i < portfolio_size; i++) {
        if (strcmp(portfolio[i].name, stock_name) == 0) {
            found_index = i;
            break;
        }
    }
    
    if (found_index != -1) {
        // Stock exists, update quantity
        portfolio[found_index].quantity += quantity;
        printf("   Database updated: Portfolio now holds %d shares of %s.\n", portfolio[found_index].quantity, stock_name);
    } else {
        // New stock, add to portfolio if there is space
        if (portfolio_size < MAX_STOCKS) {
            strncpy(portfolio[portfolio_size].name, stock_name, MAX_NAME_LEN - 1);
            portfolio[portfolio_size].name[MAX_NAME_LEN - 1] = '\0'; // Ensure null termination
            portfolio[portfolio_size].quantity = quantity;
            printf("   Database updated: Portfolio now holds %d shares of %s.\n", portfolio[portfolio_size].quantity, stock_name);
            portfolio_size++;
        } else {
            printf("   Database full! Cannot add new stock %s.\n", stock_name);
        }
    }
    
    // Call the other function as per requirement
    buy_function(stock_name);
    printf("\n"); // for better readability of test cases
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    
    // Test Case 1
    buy_stock("AAPL", 10);
    
    // Test Case 2
    buy_stock("GOOGL", 5);
    
    // Test Case 3
    buy_stock("MSFT", 20);
    
    // Test Case 4: Buying more of an existing stock
    buy_stock("AAPL", 15);
    
    // Test Case 5
    buy_stock("AMZN", 2);
    
    printf("--- Final Portfolio State ---\n");
    for (int i = 0; i < portfolio_size; i++) {
        printf("%s: %d\n", portfolio[i].name, portfolio[i].quantity);
    }
    
    return 0;
}