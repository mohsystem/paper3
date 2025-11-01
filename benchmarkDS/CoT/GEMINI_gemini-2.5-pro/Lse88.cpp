#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <vector>

// Note: This code requires the yaml-cpp library.
// You need to install it and link against it during compilation, e.g.:
// g++ -o Lse88 Lse88.cpp -lyaml-cpp
#include "yaml-cpp/yaml.h"

// Define a static map for product prices
const std::map<std::string, double> PRODUCT_PRICES = {
    {"PROD001", 10.50},
    {"PROD002", 25.00},
    {"PROD003", 5.75}
};

/**
 * @brief Parses a YAML payload, calculates the total price, and returns it as a string.
 *
 * @param yamlPayload A string containing the YAML payload with product_code and quantity.
 * @return A string representing the total price or an error message.
 */
std::string calculate_price(const std::string& yamlPayload) {
    try {
        YAML::Node data = YAML::Load(yamlPayload);

        if (!data.IsMap()) {
            return "Error: YAML payload must be a mapping.";
        }
        
        // Securely check for key existence before access
        if (!data["product_code"] || !data["quantity"]) {
            return "Error: Missing 'product_code' or 'quantity' in payload.";
        }
        
        std::string productCode;
        int quantity;
        
        try {
            productCode = data["product_code"].as<std::string>();
            quantity = data["quantity"].as<int>();
        } catch (const YAML::BadConversion& e) {
            return "Error: Type mismatch in payload. 'quantity' must be an integer.";
        }

        // Input validation
        if (quantity <= 0) {
            return "Error: Quantity must be a positive integer.";
        }

        auto it = PRODUCT_PRICES.find(productCode);
        if (it == PRODUCT_PRICES.end()) {
            return "Error: Product code not found.";
        }

        double price = it->second;
        double totalPrice = price * quantity;

        // Format output to two decimal places
        std::ostringstream stream;
        stream << std::fixed << std::setprecision(2) << totalPrice;
        return stream.str();

    } catch (const YAML::ParserException& e) {
        return std::string("Error: Invalid YAML format. ") + e.what();
    } catch (const std::exception& e) {
        return std::string("Error: An unexpected error occurred. ") + e.what();
    }
}

// Lse88 class is not idiomatic in C++, a namespace or free functions are preferred.
// But to adhere to the naming, we can wrap the main function.
class Lse88 {
public:
    static void main() {
        // Test cases
        std::vector<std::string> testCases = {
            // 1. Valid case
            "product_code: 'PROD002'\nquantity: 3",
            // 2. Product not found
            "product_code: 'PROD999'\nquantity: 1",
            // 3. Invalid YAML format (unclosed quote)
            "product_code: 'PROD001\nquantity: 5",
            // 4. Missing key ('quantity')
            "product_code: 'PROD003'",
            // 5. Invalid data type for quantity
            "product_code: 'PROD001'\nquantity: 'two'"
        };

        std::cout << "--- C++ Test Cases ---" << std::endl;
        for (size_t i = 0; i < testCases.size(); ++i) {
            std::cout << "Test Case " << i + 1 << ":\nInput:\n---\n" << testCases[i] << "\n---" << std::endl;
            std::string result = calculate_price(testCases[i]);
            std::cout << "Output: " << result << std::endl << std::endl;
        }
    }
};

int main() {
    Lse88::main();
    return 0;
}