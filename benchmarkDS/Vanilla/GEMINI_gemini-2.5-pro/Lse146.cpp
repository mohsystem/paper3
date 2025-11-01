#include <iostream>
#include <string>
#include <vector>

// Using a struct to represent an order
struct BuyOrder {
    std::string username;
    std::string stock_name;
    int stock_quantity;
};

// Simulating a database table with a global vector
std::vector<BuyOrder> ordersDatabase;

// The main class is named Lse146 as requested.
class Lse146 {
public:
    /**
     * Simulates inserting a buy order into the database.
     *
     * @param stock_name The name of the stock.
     * @param stock_quantity The quantity of the stock.
     * @param username The user placing the order.
     * @return A boolean indicating success.
     */
    static bool placeBuyOrder(const std::string& stock_name, int stock_quantity, const std::string& username) {
        // Create a new order object
        BuyOrder newOrder = {username, stock_name, stock_quantity};

        // The buy order is inserted into the database.
        ordersDatabase.push_back(newOrder);
        std::cout << "DATABASE: Successfully inserted order for " << username << std::endl;

        // The user is redirected to the profile page.
        std::cout << "SYSTEM: Redirecting user '" << username << "' to the profile page." << std::endl;

        return true;
    }
};

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1
    std::cout << "\n--- Test Case 1 ---" << std::endl;
    if (Lse146::placeBuyOrder("AAPL", 10, "john_doe")) {
        std::cout << "RESULT: Order for 10 of AAPL has been placed successfully." << std::endl;
    }

    // Test Case 2
    std::cout << "\n--- Test Case 2 ---" << std::endl;
    if (Lse146::placeBuyOrder("GOOGL", 5, "jane_smith")) {
        std::cout << "RESULT: Order for 5 of GOOGL has been placed successfully." << std::endl;
    }

    // Test Case 3
    std::cout << "\n--- Test Case 3 ---" << std::endl;
    if (Lse146::placeBuyOrder("TSLA", 20, "elon_fan")) {
        std::cout << "RESULT: Order for 20 of TSLA has been placed successfully." << std::endl;
    }

    // Test Case 4
    std::cout << "\n--- Test Case 4 ---" << std::endl;
    if (Lse146::placeBuyOrder("AMZN", 2, "john_doe")) {
        std::cout << "RESULT: Order for 2 of AMZN has been placed successfully." << std::endl;
    }

    // Test Case 5
    std::cout << "\n--- Test Case 5 ---" << std::endl;
    if (Lse146::placeBuyOrder("MSFT", 15, "bill_gates_fan")) {
        std::cout << "RESULT: Order for 15 of MSFT has been placed successfully." << std::endl;
    }

    std::cout << "\n--- Final Database State ---" << std::endl;
    for (const auto& order : ordersDatabase) {
        std::cout << "Order[User: " << order.username 
                  << ", Stock: " << order.stock_name 
                  << ", Quantity: " << order.stock_quantity << "]" << std::endl;
    }

    return 0;
}