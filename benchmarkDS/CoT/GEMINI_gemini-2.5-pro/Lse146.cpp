#include <iostream>
#include <string>
#include <vector>

// Function to trim leading/trailing whitespace from a string
std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(whitespace);
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(whitespace);
    return str.substr(first, (last - first + 1));
}

/**
 * @brief Simulates placing a stock buy order with secure practices.
 * 
 * @param username The username of the person placing the order.
 * @param stockName The name of the stock to buy.
 * @param stockQuantity The quantity of the stock to buy.
 */
void placeBuyOrder(const std::string& username, const std::string& stockName, int stockQuantity) {
    // --- Input Validation ---
    // 1. Ensure username and stockName are not empty after trimming.
    if (trim(username).empty()) {
        std::cout << "ERROR: Username cannot be empty." << std::endl;
        return;
    }
    if (trim(stockName).empty()) {
        std::cout << "ERROR: Stock name cannot be empty." << std::endl;
        return;
    }
    // 2. Ensure quantity is a positive number.
    if (stockQuantity <= 0) {
        std::cout << "ERROR: Stock quantity must be a positive number." << std::endl;
        return;
    }

    std::cout << "Processing order for user: " << username << std::endl;

    // --- Simulate Secure Database Insertion ---
    // Using prepared statements is crucial to prevent SQL Injection.
    // We simulate this by showing the query template and the bound parameters separately.
    std::cout << "DATABASE: Preparing statement: INSERT INTO buy_orders (username, stock_name, quantity) VALUES (?, ?, ?);" << std::endl;
    std::cout << "DATABASE: Binding parameter 1 (username) -> " << username << std::endl;
    std::cout << "DATABASE: Binding parameter 2 (stock_name) -> " << stockName << std::endl;
    std::cout << "DATABASE: Binding parameter 3 (quantity) -> " << stockQuantity << std::endl;
    std::cout << "DATABASE: Execution successful. Order for " << stockQuantity << " of " << stockName << " inserted." << std::endl;

    // --- Simulate Redirection ---
    std::cout << "SYSTEM: Redirecting user '" << username << "' to profile page." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

int main() {
    // Test Case 1: Valid order
    placeBuyOrder("j_doe", "AAPL", 10);

    // Test Case 2: Another valid order
    placeBuyOrder("a_smith", "GOOGL", 5);

    // Test Case 3: Invalid order with zero quantity
    placeBuyOrder("b_jones", "TSLA", 0);

    // Test Case 4: Invalid order with whitespace-only stock name
    placeBuyOrder("c_white", "   ", 100);

    // Test Case 5: Invalid order with negative quantity
    placeBuyOrder("d_green", "MSFT", -50);

    return 0;
}