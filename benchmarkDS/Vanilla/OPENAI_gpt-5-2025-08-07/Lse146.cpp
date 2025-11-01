#include <iostream>
#include <vector>
#include <string>

struct Order {
    std::string username;
    std::string stock_name;
    int stock_quantity;
};

static std::vector<Order> DATABASE;

std::string process_buy_order(const std::string& stock_name, int stock_quantity, const std::string& username) {
    Order order { username, stock_name, stock_quantity };
    DATABASE.push_back(order);
    return "/users/" + username + "/profile";
}

int main() {
    std::cout << process_buy_order("AAPL", 10, "alice") << std::endl;
    std::cout << process_buy_order("GOOGL", 5, "bob") << std::endl;
    std::cout << process_buy_order("TSLA", 2, "alice") << std::endl;
    std::cout << process_buy_order("MSFT", 100, "carol") << std::endl;
    std::cout << process_buy_order("AMZN", 7, "dave") << std::endl;
    return 0;
}