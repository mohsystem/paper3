#include <iostream>
#include <string>
#include <vector>
#include <regex>

// This is a simulation. No actual database connection is made.
// The code demonstrates the secure use of prepared statements to prevent SQL injection.
// In a real C++ application, you would use a library like SOCI, ODB, or a
// specific database connector (e.g., libpqxx for PostgreSQL, MySQL Connector/C++).

class Lse146 {
public:
    /**
     * Securely inserts a buy order into the database using a prepared statement pattern.
     * This prevents SQL injection attacks.
     *
     * @param username The user placing the order.
     * @param stockName The name of the stock.
     * @param stockQuantity The quantity of the stock to buy.
     * @return A string indicating the result of the operation.
     */
    std::string buyStock(const std::string& username, const std::string& stockName, int stockQuantity) {
        // 1. Input Validation
        const std::regex username_pattern("^[a-zA-Z0-9_]{3,20}$");
        const std::regex stock_name_pattern("^[a-zA-Z0-9.-]+$");

        if (!std::regex_match(username, username_pattern)) {
            return "Error: Invalid username format.";
        }
        if (!std::regex_match(stockName, stock_name_pattern)) {
            return "Error: Invalid stock name format.";
        }
        if (stockQuantity <= 0) {
            return "Error: Stock quantity must be a positive number.";
        }

        // 2. Use a prepared statement for security. The '?' are placeholders.
        const std::string sql = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) VALUES (?, ?, ?, NOW());";

        std::cout << "\n--- New Order ---" << std::endl;
        std::cout << "Executing secure buy order..." << std::endl;
        std::cout << "Username: " << username << std::endl;
        std::cout << "Stock: " << stockName << std::endl;
        std::cout << "Quantity: " << stockQuantity << std::endl;

        // In a real application, you would use a database library to prepare, bind, and execute.
        // Example with a hypothetical DB library:
        //
        // db_connection conn = ...;
        // db_prepared_statement stmt = conn.prepare(sql);
        // stmt.bind(1, username);
        // stmt.bind(2, stockName);
        // stmt.bind(3, stockQuantity);
        // if (stmt.execute()) {
        //     return "Success: Order placed for " + username + ". Redirecting to profile page.";
        // } else {
        //     return "Error: Failed to place order.";
        // }

        // Simulation of the prepared statement process
        std::cout << "Simulating PreparedStatement..." << std::endl;
        std::cout << "SQL Template: " << sql << std::endl;
        std::cout << "Binding parameter 1 (String): " << username << std::endl;
        std::cout << "Binding parameter 2 (String): " << stockName << std::endl;
        std::cout << "Binding parameter 3 (int): " << stockQuantity << std::endl;
        std::cout << "Simulation successful." << std::endl;

        return "Success: Order placed for " + username + ". Redirecting to profile page.";
    }
};

int main() {
    Lse146 portfolio;

    // Test Case 1: Normal valid transaction
    std::cout << portfolio.buyStock("jdoe", "GOOGL", 10) << std::endl;

    // Test Case 2: Another valid transaction
    std::cout << portfolio.buyStock("asmith", "TSLA", 5) << std::endl;

    // Test Case 3: SQL Injection attempt (should be handled safely as a literal string)
    std::cout << portfolio.buyStock("evil_user", "FAKE.STK'); DROP TABLE buy_orders; --", 1) << std::endl;

    // Test Case 4: Invalid quantity
    std::cout << portfolio.buyStock("jdoe", "MSFT", -50) << std::endl;
    
    // Test Case 5: Invalid stock name with special characters
    std::cout << portfolio.buyStock("asmith", "INV@LID", 100) << std::endl;

    return 0;
}