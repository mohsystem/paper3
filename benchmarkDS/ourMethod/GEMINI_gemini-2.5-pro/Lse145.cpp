#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>
#include <vector>

// Simulate a database table for stocks
static std::map<std::string, int> stockDatabase;
const size_t MAX_STOCK_NAME_LENGTH = 16;

/**
 * @brief A secondary function called by buy_stock.
 * In a real application, this could perform logging or auditing.
 * @param stockName The name of the stock being processed.
 */
void buy_function(const std::string& stockName) {
    // Input is considered sanitized as it comes from buy_stock
    std::cout << "[buy_function] INFO: Secondary processing for stock: " << stockName << std::endl;
}

/**
 * @brief Simulates buying a stock by validating input and adding it to an in-memory database.
 * @param stockName The name/ticker of the stock to buy.
 * @param quantity The amount of stock to buy.
 * @return true if the purchase was successful, false otherwise.
 */
bool buy_stock(std::string stockName, int quantity) {
    // Rule #1: Ensure all input is validated and sanitized
    
    // 1. Trim whitespace
    stockName.erase(0, stockName.find_first_not_of(" \t\n\r\f\v"));
    stockName.erase(stockName.find_last_not_of(" \t\n\r\f\v") + 1);

    // 2. Check for empty string
    if (stockName.empty()) {
        std::cerr << "ERROR: Stock name cannot be empty." << std::endl;
        return false;
    }

    // 3. Check length constraints
    if (stockName.length() > MAX_STOCK_NAME_LENGTH) {
        std::cerr << "ERROR: Stock name exceeds maximum length of " << MAX_STOCK_NAME_LENGTH << " characters." << std::endl;
        return false;
    }

    // 4. Validate format/characters to prevent injection attacks
    // Allows letters, numbers, dot, and hyphen.
    static const std::regex stock_name_pattern("^[a-zA-Z0-9.-]+$");
    if (!std::regex_match(stockName, stock_name_pattern)) {
        std::cerr << "ERROR: Stock name '" << stockName << "' contains invalid characters." << std::endl;
        return false;
    }
    
    // 5. Validate the quantity
    if (quantity <= 0) {
        std::cerr << "ERROR: Quantity must be a positive number." << std::endl;
        return false;
    }

    // --- Processing ---
    std::cout << "SUCCESS: Validation passed for " << quantity << " shares of " << stockName << "." << std::endl;

    // Simulate inserting/updating the record in the database
    stockDatabase[stockName] += quantity;
    std::cout << "DATABASE: Portfolio now holds " << stockDatabase[stockName] << " shares of " << stockName << "." << std::endl;

    // Call the secondary function
    buy_function(stockName);

    return true;
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid purchase
    std::cout << "\n--- Test Case 1: Valid Input ---" << std::endl;
    buy_stock("TSLA", 20);

    // Test Case 2: Invalid stock name (empty)
    std::cout << "\n--- Test Case 2: Empty Stock Name ---" << std::endl;
    buy_stock("   ", 5);

    // Test Case 3: Invalid stock name (too long)
    std::cout << "\n--- Test Case 3: Stock Name Too Long ---" << std::endl;
    buy_stock("ANOTHERREALLYLONGSTOCKNAME", 3);

    // Test Case 4: Invalid stock name (invalid characters)
    std::cout << "\n--- Test Case 4: Invalid Characters ---" << std::endl;
    buy_stock("NVDA?", 1);

    // Test Case 5: Invalid quantity (negative)
    std::cout << "\n--- Test Case 5: Invalid Quantity ---" << std::endl;
    buy_stock("AMZN", -100);

    std::cout << "\n--- Final Database State ---" << std::endl;
    for(const auto& pair : stockDatabase) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    return 0;
}