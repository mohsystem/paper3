#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <algorithm>

// Simulating a database table for stock portfolio using a std::map
std::map<std::string, int> portfolio;

/**
 * @brief This function is called by buy_stock to perform a subsequent action.
 * In a real application, this could be logging, sending a notification, etc.
 * 
 * @param stockName The name of the stock that was purchased.
 */
void buy_function(const std::string& stockName) {
    std::cout << "LOG: Post-purchase action for " << stockName << " completed." << std::endl;
}

/**
 * @brief Simulates buying a stock. It takes the stock name and quantity,
 * validates the input, "inserts" it into the simulated database,
 * and then calls a subsequent function.
 * 
 * @param stockName The ticker symbol of the stock to buy (e.g., "AAPL").
 * @param quantity The number of shares to buy.
 */
void buy_stock(std::string stockName, int quantity) {
    std::cout << "\nAttempting to buy " << quantity << " of " << stockName << "..." << std::endl;

    // --- Security: Input Validation ---
    // 1. Trim whitespace from stockName before checking if it's empty.
    stockName.erase(0, stockName.find_first_not_of(" \t\n\r"));
    stockName.erase(stockName.find_last_not_of(" \t\n\r") + 1);

    if (stockName.empty()) {
        std::cout << "Error: Stock name cannot be empty." << std::endl;
        return;
    }
    if (quantity <= 0) {
        std::cout << "Error: Quantity must be greater than zero." << std::endl;
        return;
    }

    // Sanitize input: convert to uppercase
    std::transform(stockName.begin(), stockName.end(), stockName.begin(), ::toupper);
    
    std::cout << "Inserting " << quantity << " of " << stockName << " into the database." << std::endl;

    // --- "Database" Insertion ---
    // In a real application, this would use a prepared statement to prevent SQL injection.
    // Example with a hypothetical library:
    // db_connection conn;
    // prepared_statement stmt = conn.prepare("INSERT INTO portfolio (stock_name, quantity) VALUES (?, ?) ON DUPLICATE KEY UPDATE quantity = quantity + ?;");
    // stmt.bind(1, stockName);
    // stmt.bind(2, quantity);
    // stmt.bind(3, quantity);
    // stmt.execute();

    // Simulating the database update
    portfolio[stockName] += quantity;
    std::cout << "Successfully updated portfolio for " << stockName << "." << std::endl;

    // Call the subsequent function as per the prompt
    buy_function(stockName);
}

int main() {
    std::cout << "--- Starting Stock Purchase Simulation ---" << std::endl;

    // Test Case 1: Buy a new stock
    buy_stock("GOOG", 10);

    // Test Case 2: Buy more of an existing stock
    buy_stock("GOOG", 5);

    // Test Case 3: Buy another new stock with leading/trailing spaces
    buy_stock("  MSFT  ", 20);

    // Test Case 4: Invalid quantity
    buy_stock("TSLA", -10);

    // Test Case 5: Invalid stock name
    buy_stock("", 100);

    std::cout << "\n--- Final Portfolio Status ---" << std::endl;
    if (portfolio.empty()) {
        std::cout << "Portfolio is empty." << std::endl;
    } else {
        for (const auto& pair : portfolio) {
            std::cout << "Stock: " << pair.first << ", Quantity: " << pair.second << std::endl;
        }
    }

    return 0;
}