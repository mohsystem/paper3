#include <iostream>
#include <string>
#include <map>

// Using a map to simulate a database table for stocks.
// The key is the stock name (std::string), and the value is the quantity (int).
std::map<std::string, int> stockDatabase;

/**
 * @brief Simulates the buy_order route logic.
 *
 * It takes the stock name and quantity, inserts/updates it in the database,
 * and simulates a redirect. In a real C++ web application (e.g., using Crow or Cpp-Http-Server),
 * this function would be part of a route handler.
 *
 * @param stockName The name of the stock to buy.
 * @param quantity The quantity of the stock to buy.
 */
void buyOrder(const std::string& stockName, int quantity) {
    std::cout << "\n--- Processing Buy Order ---" << std::endl;
    std::cout << "Received form data: stockName='" << stockName << "', quantity=" << quantity << std::endl;

    // Insert/Update logic for the database
    // If stock already exists, find it and add to its quantity. Otherwise, insert it.
    auto it = stockDatabase.find(stockName);
    if (it != stockDatabase.end()) {
        // Stock found, update quantity
        it->second += quantity;
    } else {
        // Stock not found, insert new entry
        stockDatabase[stockName] = quantity;
    }

    std::cout << "DATABASE: Successfully inserted/updated order for " << stockName << " with quantity " << quantity << "." << std::endl;
    std::cout << "ACTION: Redirecting user to the stock_view page..." << std::endl;
}

/**
 * @brief Simulates the stock_view page by displaying all current holdings.
 */
void viewStocks() {
    std::cout << "\n--- Displaying Stock View Page ---" << std::endl;
    std::cout << "Current Portfolio Holdings:" << std::endl;
    if (stockDatabase.empty()) {
        std::cout << "Portfolio is empty." << std::endl;
    } else {
        for (const auto& pair : stockDatabase) {
            std::cout << "- Stock: " << pair.first << ", Quantity: " << pair.second << std::endl;
        }
    }
    std::cout << "---------------------------------" << std::endl;
}

int main() {
    std::cout << "Initialized a new stock portfolio (simulated database)." << std::endl;

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