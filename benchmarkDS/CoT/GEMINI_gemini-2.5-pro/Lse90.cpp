// Note: This code requires the yaml-cpp library.
// You need to install it and link against it during compilation.
// Example compilation command:
// g++ -std=c++17 your_file_name.cpp -o your_program -lyaml-cpp

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include "yaml-cpp/yaml.h"

// Static map to store product prices
const std::map<std::string, double> productPrices = {
    {"PROD123", 19.99},
    {"PROD456", 25.50},
    {"PROD789", 0.99},
    {"PROD000", 150.00}
};

/**
 * @brief Returns the price for a given product code.
 *
 * @param productCode The code of the product.
 * @return The price of the product, or -1.0 if not found.
 */
double get_price_for_product_code(const std::string& productCode) {
    auto it = productPrices.find(productCode);
    if (it != productPrices.end()) {
        return it->second;
    }
    return -1.0;
}

/**
 * @brief Simulates a web route that calculates the total price from a YAML payload.
 *
 * @param yamlPayload A string containing the YAML request.
 * @return A YAML string with the total price or an error message.
 */
std::string calculate_price(const std::string& yamlPayload) {
    YAML::Node result;
    try {
        YAML::Node data = YAML::Load(yamlPayload);

        if (!data.IsMap()) {
            result["error"] = "YAML payload must be a mapping (dictionary)";
            std::stringstream ss;
            ss << result;
            return ss.str();
        }

        // Securely extract and validate product_code
        if (!data["product_code"] || !data["product_code"].IsScalar()) {
            result["error"] = "Missing or invalid 'product_code'";
            std::stringstream ss;
            ss << result;
            return ss.str();
        }
        std::string productCode = data["product_code"].as<std::string>();

        // Securely extract and validate quantity
        if (!data["quantity"] || !data["quantity"].IsScalar()) {
            result["error"] = "Missing or invalid 'quantity'";
            std::stringstream ss;
            ss << result;
            return ss.str();
        }
        int quantity = 0;
        try {
            quantity = data["quantity"].as<int>();
        } catch (const YAML::BadConversion& e) {
            result["error"] = "Quantity must be an integer";
            std::stringstream ss;
            ss << result;
            return ss.str();
        }

        if (quantity <= 0) {
            result["error"] = "Quantity must be a positive integer";
            std::stringstream ss;
            ss << result;
            return ss.str();
        }

        // Get unit price
        double unitPrice = get_price_for_product_code(productCode);
        if (unitPrice < 0) {
            result["error"] = "Product code not found";
            std::stringstream ss;
            ss << result;
            return ss.str();
        }

        // Calculate total price
        double totalPrice = unitPrice * quantity;
        result["total_price"] = totalPrice;

    } catch (const YAML::ParserException& e) {
        result["error"] = "Malformed YAML payload";
    } catch (const std::exception& e) {
        result["error"] = "An unexpected error occurred";
    }
    
    std::stringstream ss;
    ss << result;
    return ss.str();
}

int main() {
    // Test Case 1: Valid request
    std::string test1 = "product_code: PROD123\nquantity: 2";
    std::cout << "Test 1 Input:\n" << test1 << std::endl;
    std::cout << "Test 1 Output:\n" << calculate_price(test1) << std::endl << std::endl;
    // Expected: total_price: 39.98

    // Test Case 2: Product code not found
    std::string test2 = "product_code: UNKNOWN\nquantity: 5";
    std::cout << "Test 2 Input:\n" << test2 << std::endl;
    std::cout << "Test 2 Output:\n" << calculate_price(test2) << std::endl << std::endl;
    // Expected: error: Product code not found

    // Test Case 3: Malformed YAML (invalid syntax)
    std::string test3 = "product_code: PROD456\n  quantity: 1";
    std::cout << "Test 3 Input:\n" << test3 << std::endl;
    std::cout << "Test 3 Output:\n" << calculate_price(test3) << std::endl << std::endl;
    // Expected: error: Malformed YAML payload

    // Test Case 4: Invalid quantity (not an integer)
    std::string test4 = "product_code: PROD789\nquantity: 'three'";
    std::cout << "Test 4 Input:\n" << test4 << std::endl;
    std::cout << "Test 4 Output:\n" << calculate_price(test4) << std::endl << std::endl;
    // Expected: error: Quantity must be an integer

    // Test Case 5: Missing product_code key
    std::string test5 = "quantity: 10";
    std::cout << "Test 5 Input:\n" << test5 << std::endl;
    std::cout << "Test 5 Output:\n" << calculate_price(test5) << std::endl << std::endl;
    // Expected: error: Missing or invalid 'product_code'

    return 0;
}