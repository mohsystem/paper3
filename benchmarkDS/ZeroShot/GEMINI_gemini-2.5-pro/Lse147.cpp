#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

// A simple struct to represent a stock order
struct Order {
    std::string stockName;
    int quantity;
};

// In-memory vector to act as a simple database
std::vector<Order> database;

// Helper function to check if a string is alphanumeric
bool isAlphanumeric(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isalnum);
}

// Helper function to check if a string is only whitespace
bool isWhitespace(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isspace);
}

/**
 * @brief Simulates buying a stock by validating input and adding it to an in-memory database.
 * This function demonstrates secure coding practices like input validation.
 *
 * @param stockName The name of the stock (e.g., "AAPL"). Must be alphanumeric and not empty.
 * @param quantity The number of shares to buy. Must be a positive integer.
 * @return A string indicating the result of the operation.
 */
std::string buy_order(const std::string& stockName, int quantity) {
    // 1. Security: Validate stock name - ensure it's not empty or just whitespace.
    if (stockName.empty() || isWhitespace(stockName)) {
        return "Error: Stock name cannot be empty.";
    }
    
    // 2. Security: Sanitize stock name - ensure it contains only allowed characters.
    // This helps prevent injection attacks in a real database scenario.
    if (!isAlphanumeric(stockName)) {
        return "Error: Stock name must be alphanumeric.";
    }

    // 3. Security: Validate quantity - ensure it's a positive number.
    if (quantity <= 0) {
        return "Error: Quantity must be greater than zero.";
    }

    // 4. "Insert" into the database if all checks pass.
    database.push_back({stockName, quantity});

    // 5. Simulate redirection by returning a success message.
    return "SUCCESS: Order for " + std::to_string(quantity) + " shares of " + stockName + " placed. Redirecting to stock_view.";
}


int main() {
    // --- Test Cases ---
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid order
    std::cout << "1. " << buy_order("AAPL", 100) << std::endl;

    // Test Case 2: Invalid quantity (zero)
    std::cout << "2. " << buy_order("GOOG", 0) << std::endl;

    // Test Case 3: Invalid stock name (contains special characters)
    std::cout << "3. " << buy_order("MSFT-X", 50) << std::endl;

    // Test Case 4: Invalid stock name (empty)
    std::cout << "4. " << buy_order("  ", 200) << std::endl;
    
    // Test Case 5: Valid order after some failures
    std::cout << "5. " << buy_order("TSLA", 25) << std::endl;

    // --- Display Database Content ---
    std::cout << "\n--- Current Database State ---" << std::endl;
    if (database.empty()) {
        std::cout << "Database is empty." << std::endl;
    } else {
        for (size_t i = 0; i < database.size(); ++i) {
            std::cout << "Entry " << i + 1 << ": Stock=" << database[i].stockName 
                      << ", Quantity=" << database[i].quantity << std::endl;
        }
    }
    
    return 0;
}