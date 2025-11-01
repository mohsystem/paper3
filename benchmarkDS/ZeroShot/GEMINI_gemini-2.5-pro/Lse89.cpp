/*
 * IMPORTANT: This code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from the official repository:
 * https://github.com/yhirose/cpp-httplib
 * and place it in the same directory or in your include path.
 *
 * Compilation command:
 * g++ -o Lse89_cpp Lse89.cpp -I. -pthread -std=c++17
 */
#include "httplib.h"
#include <iostream>
#include <map>
#include <string>
#include <optional>
#include <iomanip>
#include <sstream>

// The main class 'Lse89' is emulated through namespace and functions
namespace Lse89 {

    // In-memory database for product prices
    const std::map<std::string, double> productPrices = {
        {"A123", 10.50},
        {"B456", 25.00},
        {"C789", 5.75},
        {"D101", 99.99}
    };

    /**
     * Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return An optional containing the price, or nullopt if not found.
     */
    std::optional<double> getPrice(const std::string& productCode) {
        auto it = productPrices.find(productCode);
        if (it != productPrices.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void run_server() {
        httplib::Server svr;

        svr.Get("/price", [](const httplib::Request& req, httplib::Response& res) {
            if (!req.has_param("product_code") || !req.has_param("quantity")) {
                res.status = 400;
                res.set_content("{\"error\":\"Missing product_code or quantity parameter\"}", "application/json");
                return;
            }

            std::string productCode = req.get_param_value("product_code");
            std::string quantityStr = req.get_param_value("quantity");

            auto priceOpt = getPrice(productCode);
            if (!priceOpt) {
                res.status = 404;
                res.set_content("{\"error\":\"Product code not found\"}", "application/json");
                return;
            }

            try {
                long quantity = std::stol(quantityStr);
                if (quantity <= 0) {
                    res.status = 400;
                    res.set_content("{\"error\":\"Quantity must be a positive integer\"}", "application/json");
                    return;
                }

                double totalPrice = *priceOpt * quantity;
                
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << "{\"product_code\":\"" << productCode 
                   << "\", \"quantity\":" << quantity << ", \"total_price\":" << totalPrice << "}";
                
                res.status = 200;
                res.set_content(ss.str(), "application/json");

            } catch (const std::invalid_argument& ia) {
                res.status = 400;
                res.set_content("{\"error\":\"Quantity must be a valid integer\"}", "application/json");
            } catch (const std::out_of_range& oor) {
                res.status = 400;
                res.set_content("{\"error\":\"Quantity is out of range\"}", "application/json");
            }
        });

        int port = 5000;
        std::cout << "C++ server starting on port " << port << std::endl;
        svr.listen("0.0.0.0", port);
    }
} // namespace Lse89

int main() {
    std::cout << "--- Test Cases (use curl or a browser) ---" << std::endl;
    std::cout << "1. Valid: curl \"http://localhost:5000/price?product_code=A123&quantity=5\"" << std::endl;
    std::cout << "2. Not Found: curl \"http://localhost:5000/price?product_code=X999&quantity=2\"" << std::endl;
    std::cout << "3. Invalid Quantity (text): curl \"http://localhost:5000/price?product_code=B456&quantity=abc\"" << std::endl;
    std::cout << "4. Invalid Quantity (negative): curl \"http://localhost:5000/price?product_code=C789&quantity=-3\"" << std::endl;
    std::cout << "5. Missing Parameter: curl \"http://localhost:5000/price?product_code=A123\"" << std::endl;

    Lse89::run_server();
    return 0;
}