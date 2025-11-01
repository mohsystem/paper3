/*
 * This code requires the 'cpp-httplib' single-header library.
 * Download 'httplib.h' from https://github.com/yhirose/cpp-httplib
 * and place it in the same directory as this file or in your include path.
 *
 * Compilation command: g++ -o server Lse89.cpp -pthread
 */
#include "httplib.h"
#include <iostream>
#include <map>
#include <string>
#include <iomanip>
#include <sstream>

// The map to store product codes and their prices
const std::map<std::string, double> productPrices = {
    {"P101", 10.50},
    {"P205", 25.00},
    {"P330", 5.75},
    {"P412", 99.99},
    {"P555", 1.25}
};

/**
 * @brief Returns the price for a given product code.
 * 
 * @param productCode The code of the product.
 * @return The price of the product, or 0.0 if not found.
 */
double getPrice(const std::string& productCode) {
    auto it = productPrices.find(productCode);
    if (it != productPrices.end()) {
        return it->second;
    }
    return 0.0;
}

void Lse89() {
    httplib::Server svr;

    // Defines the route for /<product_code>/<quantity>
    // The regex captures an alphanumeric product code and a numeric quantity
    svr.Get(R"(/([A-Z0-9]+)/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string productCode = req.matches[1];
        int quantity = 0;
        
        try {
            quantity = std::stoi(req.matches[2]);
        } catch (const std::invalid_argument& ia) {
            res.status = 400;
            res.set_content("{\"error\": \"Invalid quantity format.\"}", "application/json");
            return;
        }

        double unitPrice = getPrice(productCode);
        
        if (unitPrice > 0) {
            if (quantity > 0) {
                double totalPrice = unitPrice * quantity;
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2)
                   << "{\"product_code\": \"" << productCode << "\", "
                   << "\"quantity\": " << quantity << ", "
                   << "\"unit_price\": " << unitPrice << ", "
                   << "\"total_price\": " << totalPrice << "}";
                res.set_content(ss.str(), "application/json");
            } else {
                res.status = 400;
                res.set_content("{\"error\": \"Quantity must be a positive integer.\"}", "application/json");
            }
        } else {
            res.status = 404;
            res.set_content("{\"error\": \"Product code not found.\"}", "application/json");
        }
    });

    // Default handler for malformed URLs
    svr.set_error_handler([](const httplib::Request&, httplib::Response& res) {
        const char* fmt = "{\"error\": \"Invalid URL format or type. Use /<product_code>/<integer_quantity>\"}";
        res.set_content(fmt, "application/json");
        res.status = 404;
    });

    std::cout << "C++ server started on port 5000." << std::endl;
    std::cout << "--- Test Cases ---" << std::endl;
    std::cout << "1. Valid: curl http://localhost:5000/P101/10" << std::endl;
    std::cout << "2. Valid: curl http://localhost:5000/P412/3" << std::endl;
    std::cout << "3. Invalid Product: curl http://localhost:5000/P999/5" << std::endl;
    std::cout << "4. Invalid Quantity (text): curl http://localhost:5000/P205/abc" << std::endl;
    std::cout << "5. Invalid Quantity (zero): curl http://localhost:5000/P330/0" << std::endl;
    
    svr.listen("0.0.0.0", 5000);
}

int main() {
    Lse89();
    return 0;
}