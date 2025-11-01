#include <iostream>
#include <string>
#include <map>

// Simulating a database table for stock holdings
std::map<std::string, int> portfolio;

/**
 * This function is called by buy_stock to perform additional processing.
 * It takes the stock name as an argument.
 * @param stockName The name of the stock.
 */
void buy_function(const std::string& stockName) {
    std::cout << "-> buy_function: Performing additional checks/logging for " << stockName << "..." << std::endl;
}

/**
 * Simulates buying a stock and inserting it into a database.
 * This function takes the stock name and quantity and updates the portfolio.
 * It then calls the buy_function.
 * @param stockName The name of the stock to buy.
 * @param quantity The quantity of the stock to buy.
 */
void buy_stock(const std::string& stockName, int quantity) {
    std::cout << "Attempting to buy " << quantity << " shares of " << stockName << "." << std::endl;
    
    // Simulate inserting/updating the database
    portfolio[stockName] += quantity;
    std::cout << "   Database updated: Portfolio now holds " << portfolio[stockName] << " shares of " << stockName << "." << std::endl;
    
    // Call the other function as per requirement
    buy_function(stockName);
    std::cout << std::endl; // for better readability of test cases
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    
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
    
    std::cout << "--- Final Portfolio State ---" << std::endl;
    for (const auto& pair : portfolio) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    return 0;
}