#include <iostream>
#include <string>
#include <regex>
#include <vector>

// Constants for validation
const int MAX_USERNAME_LENGTH = 50;
const int MAX_STOCK_NAME_LENGTH = 10;

/**
 * Simulates inserting a buy order into a database using a secure, parameterized query pattern.
 */
void insertBuyOrderIntoDB(const std::string& username, const std::string& stockName, int quantity) {
    // This is a simulation of a secure database insertion.
    // In a real application, this would use a library like SOCI, OTL, or a vendor-specific
    // API with prepared statements to prevent SQL injection.
    std::cout << "DATABASE: Preparing to execute INSERT INTO buy_orders (username, stock_name, quantity) VALUES (?, ?, ?)" << std::endl;
    std::cout << "DATABASE: Binding parameter 1 (username) -> " << username << std::endl;
    std::cout << "DATABASE: Binding parameter 2 (stock_name) -> " << stockName << std::endl;
    std::cout << "DATABASE: Binding parameter 3 (quantity) -> " << quantity << std::endl;
    std::cout << "DATABASE: Order for " << quantity << " of " << stockName << " by " << username << " inserted successfully." << std::endl;
}

/**
 * Simulates redirecting the user to their profile page.
 */
void redirectToProfile(const std::string& username) {
    std::cout << "REDIRECT: Now redirecting to /profile/" << username << std::endl;
}

/**
 * Processes a stock buy order after validating the inputs.
 * @return true if the order was processed successfully, false otherwise.
 */
bool processBuyOrder(const std::string& username, const std::string& stockName, int stockQuantity) {
    // Rule #1: Input Validation
    // Simple regex: alphanumeric, 3-50 chars
    const std::regex username_pattern("^[a-zA-Z0-9]{3," + std::to_string(MAX_USERNAME_LENGTH) + "}$");
    // Simple regex: uppercase letters, 1-10 chars
    const std::regex stock_name_pattern("^[A-Z]{1," + std::to_string(MAX_STOCK_NAME_LENGTH) + "}$");
    
    if (!std::regex_match(username, username_pattern)) {
        std::cerr << "Error: Invalid username format or length." << std::endl;
        return false;
    }
    
    if (!std::regex_match(stockName, stock_name_pattern)) {
        std::cerr << "Error: Invalid stock name format. Must be 1-" << MAX_STOCK_NAME_LENGTH << " uppercase letters." << std::endl;
        return false;
    }
    
    if (stockQuantity <= 0) {
        std::cerr << "Error: Stock quantity must be a positive number." << std::endl;
        return false;
    }
    
    std::cout << "Input validation passed." << std::endl;

    // Simulate database insertion
    insertBuyOrderIntoDB(username, stockName, stockQuantity);

    // Simulate redirection
    redirectToProfile(username);
    
    return true;
}

int main() {
    std::cout << "--- Test Case 1: Valid Order ---" << std::endl;
    processBuyOrder("testuser123", "GOOGL", 10);
    std::cout << std::endl;

    std::cout << "--- Test Case 2: Invalid Username (too short) ---" << std::endl;
    processBuyOrder("ab", "AAPL", 5);
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Invalid Stock Name (number) ---" << std::endl;
    processBuyOrder("anotheruser", "TSLA1", 20);
    std::cout << std::endl;

    std::cout << "--- Test Case 4: Invalid Quantity (zero) ---" << std::endl;
    processBuyOrder("validuser", "TSLA", 0);
    std::cout << std::endl;

    std::cout << "--- Test Case 5: Valid Order 2 ---" << std::endl;
    processBuyOrder("traderjane", "MSFT", 50);
    std::cout << std::endl;
    
    return 0;
}