// To compile: g++ -std=c++17 -o app your_file_name.cpp -I/path/to/crow/include -lboost_system -pthread
// Requires the Crow C++ framework. Download crow_all.h or install it.
#define CROW_MAIN
#include "crow_all.h" // Assumes crow_all.h is available in the include path

#include <iostream>
#include <string>
#include <map>
#include <optional>

// Product price database
const std::map<std::string, double> productPrices = {
    {"PROD001", 19.99},
    {"PROD002", 25.50},
    {"PROD003", 5.75},
    {"PROD004", 99.99}
};

/**
 * @brief Returns the price for a given product code.
 *
 * @param productCode The code of the product.
 * @return std::optional<double> The price if found, otherwise std::nullopt.
 */
std::optional<double> getProductPrice(const std::string& productCode) {
    auto it = productPrices.find(productCode);
    if (it != productPrices.end()) {
        return it->second;
    }
    return std::nullopt;
}

int main() {
    crow::SimpleApp app;

    CROW_ROUTE(app, "/price")
    ([](const crow::request& req) {
        char* code_param = req.url_params.get("code");
        char* quantity_param = req.url_params.get("quantity");

        if (!code_param || !quantity_param) {
            crow::json::wvalue error;
            error["error"] = "Missing 'code' or 'quantity' parameter";
            return crow::response(400, error);
        }

        std::string productCode(code_param);
        std::string quantityStr(quantity_param);

        auto priceOpt = getProductPrice(productCode);
        if (!priceOpt) {
            crow::json::wvalue error;
            error["error"] = "Product code not found";
            return crow::response(404, error);
        }

        unsigned long quantity;
        try {
            size_t pos;
            quantity = std::stoul(quantityStr, &pos);
            // Ensure the whole string was a valid number
            if (pos != quantityStr.length()) {
                throw std::invalid_argument("Trailing characters");
            }
            if (quantity == 0 || quantity > 10000) {
                 throw std::out_of_range("Quantity out of valid range");
            }
        } catch (const std::invalid_argument&) {
            crow::json::wvalue error;
            error["error"] = "Quantity must be a valid positive integer";
            return crow::response(400, error);
        } catch (const std::out_of_range&) {
            crow::json::wvalue error;
            error["error"] = "Invalid quantity. Must be between 1 and 10000.";
            return crow::response(400, error);
        }

        double unit_price = *priceOpt;
        double total_price = unit_price * quantity;

        crow::json::wvalue response;
        response["product_code"] = productCode;
        response["quantity"] = quantity;
        response["unit_price"] = unit_price;
        response["total_price"] = total_price;

        return crow::response(200, response);
    });
    
    std::cout << "Server starting on port 5000\n";
    std::cout << "\n--- Test Cases ---\n";
    std::cout << "Run these commands in another terminal:\n";
    std::cout << "1. Valid product and quantity: curl \"http://localhost:5000/price?code=PROD001&quantity=10\"\n";
    std::cout << "2. Another valid product: curl \"http://localhost:5000/price?code=PROD004&quantity=2\"\n";
    std::cout << "3. Invalid product code: curl \"http://localhost:5000/price?code=INVALID&quantity=5\"\n";
    std::cout << "4. Invalid quantity (non-numeric): curl \"http://localhost:5000/price?code=PROD002&quantity=abc\"\n";
    std::cout << "5. Missing quantity parameter: curl \"http://localhost:5000/price?code=PROD003\"\n";
    std::cout << "--------------------\n\n";

    app.port(5000).multithreaded().run();

    return 0;
}