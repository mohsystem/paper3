#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct to represent an order
typedef struct {
    char* username;
    char* stock_name;
    int stock_quantity;
} BuyOrder;

// Simulating a database with a dynamic array of structs
BuyOrder* ordersDatabase = NULL;
int orderCount = 0;
int databaseCapacity = 0;

// Function to free memory allocated for the database
void cleanupDatabase() {
    if (ordersDatabase != NULL) {
        for (int i = 0; i < orderCount; i++) {
            free(ordersDatabase[i].username);
            free(ordersDatabase[i].stock_name);
        }
        free(ordersDatabase);
        ordersDatabase = NULL;
        orderCount = 0;
        databaseCapacity = 0;
    }
}

/**
 * Simulates inserting a buy order into the database.
 *
 * @param stock_name The name of the stock.
 * @param stock_quantity The quantity of the stock.
 * @param username The user placing the order.
 * @return 1 for success, 0 for failure.
 */
int placeBuyOrder(const char* stock_name, int stock_quantity, const char* username) {
    // Resize the database if it's full
    if (orderCount >= databaseCapacity) {
        int newCapacity = (databaseCapacity == 0) ? 4 : databaseCapacity * 2;
        BuyOrder* temp = realloc(ordersDatabase, newCapacity * sizeof(BuyOrder));
        if (temp == NULL) {
            fprintf(stderr, "Failed to allocate memory for database.\n");
            return 0; // Failure
        }
        ordersDatabase = temp;
        databaseCapacity = newCapacity;
    }

    // Allocate memory for the strings in the new order
    char* user_copy = malloc(strlen(username) + 1);
    char* stock_copy = malloc(strlen(stock_name) + 1);

    if (user_copy == NULL || stock_copy == NULL) {
        fprintf(stderr, "Failed to allocate memory for order details.\n");
        free(user_copy); // free if one of them was allocated
        free(stock_copy);
        return 0; // Failure
    }

    strcpy(user_copy, username);
    strcpy(stock_copy, stock_name);
    
    // The buy order is inserted into the database.
    ordersDatabase[orderCount].username = user_copy;
    ordersDatabase[orderCount].stock_name = stock_copy;
    ordersDatabase[orderCount].stock_quantity = stock_quantity;
    orderCount++;
    
    printf("DATABASE: Successfully inserted order for %s\n", username);

    // The user is redirected to the profile page.
    printf("SYSTEM: Redirecting user '%s' to the profile page.\n", username);

    return 1; // Success
}

// In C, there is no concept of a main class like "Lse146".
int main() {
    printf("--- Running 5 Test Cases ---\n");
    
    // Test Case 1
    printf("\n--- Test Case 1 ---\n");
    if (placeBuyOrder("AAPL", 10, "john_doe")) {
        printf("RESULT: Order for 10 of AAPL has been placed successfully.\n");
    }

    // Test Case 2
    printf("\n--- Test Case 2 ---\n");
    if (placeBuyOrder("GOOGL", 5, "jane_smith")) {
        printf("RESULT: Order for 5 of GOOGL has been placed successfully.\n");
    }

    // Test Case 3
    printf("\n--- Test Case 3 ---\n");
    if (placeBuyOrder("TSLA", 20, "elon_fan")) {
        printf("RESULT: Order for 20 of TSLA has been placed successfully.\n");
    }

    // Test Case 4
    printf("\n--- Test Case 4 ---\n");
    if (placeBuyOrder("AMZN", 2, "john_doe")) {
        printf("RESULT: Order for 2 of AMZN has been placed successfully.\n");
    }

    // Test Case 5
    printf("\n--- Test Case 5 ---\n");
    if (placeBuyOrder("MSFT", 15, "bill_gates_fan")) {
        printf("RESULT: Order for 15 of MSFT has been placed successfully.\n");
    }
    
    printf("\n--- Final Database State ---\n");
    for (int i = 0; i < orderCount; i++) {
        printf("Order[User: %s, Stock: %s, Quantity: %d]\n", 
               ordersDatabase[i].username, 
               ordersDatabase[i].stock_name, 
               ordersDatabase[i].stock_quantity);
    }

    // Clean up all dynamically allocated memory before exiting
    cleanupDatabase();

    return 0;
}