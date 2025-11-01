// Note: This code requires the yaml-cpp library.
// On Debian/Ubuntu: sudo apt-get install libyaml-cpp-dev
// Compilation: g++ -std=c++11 -o cpp_app your_file_name.cpp -lyaml-cpp

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

// Include the yaml-cpp header. The exact path might vary.
#include "yaml-cpp/yaml.h"

static const std::map<std::string, double> PRODUCT_PRICES = {
    {"PROD_A", 19.99},
    {"PROD_B", 250.00},
    {"PROD_C", 0.50}
};
static const int MAX_QUANTITY = 1000000;

/**
 * Parses a YAML payload, calculates the total price, and returns it as a string.
 *
 * @param yamlPayload A constant reference to a string containing the YAML payload.
 * @return A string representing the total price or an error message.
 */
std::string calculate_price(const std::string& yamlPayload) {
    if (yamlPayload.empty()) {
        return "Error: YAML payload cannot be empty.";
    }

    YAML::Node data;
    try {
        data = YAML::Load(yamlPayload);
    } catch (const YAML::ParserException& e) {
        // In a real application, log e.what() but return a generic message.
        return "Error: Failed to parse YAML.";
    }

    if (!data.IsMap()) {
        return "Error: Invalid YAML format. Root must be a mapping.";
    }

    // Validate product_code
    if (!data["product_code"] || !data["product_code"].IsScalar()) {
        return "Error: Missing or invalid 'product_code'.";
    }
    std::string productCode;
    try {
        productCode = data["product_code"].as<std::string>();
    } catch(const YAML::Exception& e) {
        return "Error: 'product_code' has invalid format.";
    }

    if (productCode.empty()) {
        return "Error: 'product_code' cannot be empty.";
    }

    // Validate quantity
    if (!data["quantity"]) {
        return "Error: Missing 'quantity'.";
    }

    int quantity;
    try {
        quantity = data["quantity"].as<int>();
    } catch (const YAML::BadConversion& e) {
        return "Error: 'quantity' must be an integer.";
    }

    // Business logic validation
    auto it = PRODUCT_PRICES.find(productCode);
    if (it == PRODUCT_PRICES.end()) {
        return "Error: Product not found.";
    }
    if (quantity < 0) {
        return "Error: Quantity cannot be negative.";
    }
    if (quantity > MAX_QUANTITY) {
        std::ostringstream oss;
        oss << "Error: Quantity exceeds maximum limit of " << MAX_QUANTITY;
        return oss.str();
    }

    double price = it->second;
    double totalPrice = price * quantity;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << totalPrice;
    return oss.str();
}

void run_test_case(int caseNum, const std::string& input) {
    std::cout << "--- Test Case " << caseNum << " ---" << std::endl;
    std::cout << "Input:\n" << input << std::endl;
    std::cout << "Output: " << calculate_price(input) << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test Case 1: Valid input
    std::string test1 = "product_code: 'PROD_A'\nquantity: 10";
    run_test_case(1, test1);

    // Test Case 2: Invalid YAML syntax
    std::string test2 = "product_code: 'PROD_B\n  quantity: 5";
    run_test_case(2, test2);

    // Test Case 3: Unknown product code
    std::string test3 = "product_code: 'PROD_X'\nquantity: 100";
    run_test_case(3, test3);

    // Test Case 4: Negative quantity
    std::string test4 = "product_code: 'PROD_C'\nquantity: -5";
    run_test_case(4, test4);

    // Test Case 5: Missing quantity key
    std::string test5 = "product_code: 'PROD_A'";
    run_test_case(5, test5);

    return 0;
}