#include <iostream>
#include <string>
#include <vector>

// Forward declaration is good practice if buy_function is defined after its first use.
void buy_function(const std::string& stockName);

/**
 * @brief Simulates buying a stock and securely inserting the transaction into a database.
 * 
 * This function demonstrates input validation and the use of parameterized queries
 * to prevent SQL injection vulnerabilities.
 * 
 * @param stockName The name of the stock to buy (e.g., "AAPL").
 * @param quantity The number of shares to buy.
 */
void buy_stock(const std::string& stockName, int quantity) {
    // --- Security: Input Validation ---
    // 1. Check for empty or whitespace-only strings.
    // 2. Ensure quantity is a positive number.
    if (stockName.empty() || stockName.find_first_not_of(" \t\n\r") == std::string::npos) {
        std::cout << "Error: Stock name cannot be empty or just whitespace.\n" << std::endl;
        return;
    }
    if (quantity <= 0) {
        std::cout << "Error: Quantity must be a positive number.\n" << std::endl;
        return;
    }

    std::cout << "Processing purchase of " << quantity << " shares of " << stockName << std::endl;

    // --- Security: Using Parameterized Queries (Prepared Statements) ---
    // This is the standard way to prevent SQL injection. Database libraries
    // provide mechanisms to bind parameters safely.

    // An insecure concatenation would be vulnerable:
    // std::string query = "INSERT INTO stocks (name, quantity) VALUES ('" + stockName + "', " + std::to_string(quantity) + ")";
    
    // The secure, parameterized approach (simulated):
    std::cout << "Simulating secure database insertion..." << std::endl;
    std::cout << "  Prepared Statement: INSERT INTO stocks (name, quantity) VALUES (?, ?)" << std::endl;
    std::cout << "  Binding parameter 1 (string): " << stockName << std::endl;
    std::cout << "  Binding parameter 2 (int): " << quantity << std::endl;
    std::cout << "  Execution successful." << std::endl;

    // Call the other function as required.
    buy_function(stockName);
    std::cout << std::endl; // Add a newline for cleaner test output.
}

/**
 * @brief Simulates some secondary logic that runs after a stock purchase is recorded.
 * @param stockName The name of the stock.
 */
void buy_function(const std::string& stockName) {
    std::cout << "-> buy_function: Executing secondary logic for stock: " << stockName << std::endl;
}

int main() {
    std::cout << "--- Running C++ Test Cases ---\n" << std::endl;

    // Test Case 1: Valid input
    buy_stock("AAPL", 10);

    // Test Case 2: Another valid input
    buy_stock("GOOG", 5);

    // Test Case 3: Invalid quantity
    buy_stock("MSFT", 0);

    // Test Case 4: Invalid stock name (empty)
    buy_stock("", 100);

    // Test Case 5: Invalid stock name (whitespace)
    buy_stock("  \t  ", 20);
    
    return 0;
}