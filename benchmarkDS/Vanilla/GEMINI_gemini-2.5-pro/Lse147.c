#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STOCKS 100
#define MAX_NAME_LEN 50

// Using a struct to represent a stock entry
typedef struct {
    char name[MAX_NAME_LEN];
    int quantity;
} Stock;

// An array of structs to simulate a database table
Stock stockDatabase[MAX_STOCKS];
int stockCount = 0; // Tracks the number of unique stocks in the database

/**
 * @brief Simulates the buy_order route logic.
 *
 * It takes the stock name and quantity and inserts/updates it in the database.
 * C is not typically used for modern web backends, but this simulates the core
 * data handling logic of such a system (e.g., a CGI script).
 *
 * @param stockName The name of the stock to buy.
 * @param quantity The quantity of the stock to buy.
 */
void buyOrder(const char* stockName, int quantity) {
    printf("\n--- Processing Buy Order ---\n");
    printf("Received form data: stockName='%s', quantity=%d\n", stockName, quantity);

    int found_index = -1;
    // Search if the stock already exists in our database
    for (int i = 0; i < stockCount; i++) {
        if (strcmp(stockDatabase[i].name, stockName) == 0) {
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Stock exists, update its quantity
        stockDatabase[found_index].quantity += quantity;
    } else {
        // Stock does not exist, add a new entry if there is space
        if (stockCount < MAX_STOCKS) {
            strncpy(stockDatabase[stockCount].name, stockName, MAX_NAME_LEN - 1);
            stockDatabase[stockCount].name[MAX_NAME_LEN - 1] = '\0'; // Ensure null-termination
            stockDatabase[stockCount].quantity = quantity;
            stockCount++;
        } else {
            printf("DATABASE ERROR: Database is full. Cannot add new stock.\n");
            return;
        }
    }

    printf("DATABASE: Successfully inserted/updated order for %s with quantity %d.\n", stockName, quantity);
    printf("ACTION: Redirecting user to the stock_view page...\n");
}

/**
 * @brief Simulates the stock_view page by displaying all current holdings.
 */
void viewStocks() {
    printf("\n--- Displaying Stock View Page ---\n");
    printf("Current Portfolio Holdings:\n");
    if (stockCount == 0) {
        printf("Portfolio is empty.\n");
    } else {
        for (int i = 0; i < stockCount; i++) {
            printf("- Stock: %s, Quantity: %d\n", stockDatabase[i].name, stockDatabase[i].quantity);
        }
    }
    printf("---------------------------------\n");
}

int main() {
    printf("Initialized a new stock portfolio (simulated database).\n");

    // --- 5 Test Cases ---

    // Test Case 1: Buy a new stock
    buyOrder("AAPL", 10);

    // Test Case 2: Buy another new stock
    buyOrder("GOOGL", 5);

    // Test Case 3: Buy more of an existing stock
    buyOrder("AAPL", 15);

    // Test Case 4: Buy a stock with a different name
    buyOrder("TSLA", 8);

    // Test Case 5: Buy a small quantity of another stock
    buyOrder("MSFT", 2);

    // Display the final state of the portfolio, simulating the stock_view page
    viewStocks();

    return 0;
}