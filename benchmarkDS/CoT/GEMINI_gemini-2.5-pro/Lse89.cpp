/*
 * This code requires the cpp-httplib library.
 * It's a header-only library, so you just need to download httplib.h
 * and place it in the same directory or in your include path.
 *
 * You can get it from: https://github.com/yhirose/cpp-httplib
 *
 * Compile command (assuming httplib.h is in the same directory):
 * g++ -o Lse89 Lse89.cpp -std=c++17 -lpthread
 */
#define CPPHTTPLIB_OPENSSL_SUPPORT // If you need HTTPS support, also link -lssl -lcrypto
#include "httplib.h" 
#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <iomanip>

// Define product prices in a global map
const std::map<std::string, double> productPrices = {
    {"P100", 10.50},
    {"P200", 25.00},
    {"P300", 5.75},
    {"P400", 19.99}
};

/**
 * @brief Returns the price for a given product code.
 * @param productCode The code of the product.
 * @return The price of the product.
 * @throws std::out_of_range if the product code is not found.
 */
double getProductPrice(const std::string& productCode) {
    // .at() throws std::out_of_range if key doesn't exist
    return productPrices.at(productCode);
}

void runTests() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    // Test Case 1: Valid product
    try {
        std::cout << "Test 1: P100 Price: $" << getProductPrice("P100") << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Test 1: Error - " << e.what() << std::endl;
    }

    // Test Case 2: Another valid product
    try {
        std::cout << "Test 2: P400 Price: $" << getProductPrice("P400") << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Test 2: Error - " << e.what() << std::endl;
    }

    // Test Case 3: Invalid product
    try {
        getProductPrice("P999");
    } catch (const std::out_of_range& e) {
        std::cout << "Test 3: P999 Price: Product code not found as expected." << std::endl;
    }
    
    // Test Case 4: Empty string product code
    try {
        getProductPrice("");
    } catch (const std::out_of_range& e) {
        std::cout << "Test 4: '' Price: Product code not found as expected." << std::endl;
    }

    // Test Case 5: A different invalid product
    try {
        getProductPrice("INVALID");
    } catch (const std::out_of_range& e) {
        std::cout << "Test 5: 'INVALID' Price: Product code not found as expected." << std::endl;
    }
    
    std::cout << "--------------------------\n" << std::endl;
}


int main() {
    // Run test cases before starting the server
    runTests();

    // Instantiate the server
    httplib::Server svr;

    // Define the route /calculate_price
    svr.Get("/calculate_price", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("code") || !req.has_param("quantity")) {
            res.status = 400; // Bad Request
            res.set_content("Error: 'code' and 'quantity' parameters are required.", "text/plain");
            return;
        }

        std::string productCode = req.get_param_value("code");
        std::string quantityStr = req.get_param_value("quantity");
        
        double price;
        try {
            price = getProductPrice(productCode);
        } catch (const std::out_of_range& e) {
            res.status = 404; // Not Found
            res.set_content("Error: Product code '" + productCode + "' not found.", "text/plain");
            return;
        }

        int quantity;
        try {
            quantity = std::stoi(quantityStr);
            if (quantity <= 0) {
                 res.status = 400; // Bad Request
                 res.set_content("Error: Quantity must be a positive integer.", "text/plain");
                 return;
            }
        } catch (const std::invalid_argument& e) {
            res.status = 400; // Bad Request
            res.set_content("Error: 'quantity' must be a valid integer.", "text/plain");
            return;
        } catch (const std::out_of_range& e) {
            res.status = 400; // Bad Request
            res.set_content("Error: 'quantity' value is out of range.", "text/plain");
            return;
        }

        double totalPrice = price * quantity;

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2)
           << "Total price for " << quantity << " of " << productCode
           << " is $" << totalPrice;
        
        res.status = 200; // OK
        res.set_content(ss.str(), "text/plain");
    });

    int port = 5000;
    std::cout << "Server starting on port " << port << std::endl;
    std::cout << "Try accessing: http://localhost:5000/calculate_price?code=P300&quantity=10" << std::endl;
    
    // Listen on all available interfaces
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Failed to start server on port " << port << std::endl;
        return 1;
    }

    return 0;
}