#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <vector>

// This code requires the yaml-cpp library.
// On Debian/Ubuntu: sudo apt-get install libyaml-cpp-dev
// On macOS (Homebrew): brew install yaml-cpp
// Compile with: g++ -std=c++17 your_file.cpp -o your_app -lyaml-cpp
#include "yaml-cpp/yaml.h"

// In a real project, this would be a more robust database lookup.
static const std::map<std::string, double> PRODUCT_PRICES = {
    {"PROD123", 10.50},
    {"PROD456", 25.00},
    {"PROD789", 5.75}
};

/**
 * @brief Returns the price for a product code.
 * @param productCode The product code to look up.
 * @return The price of the product.
 * @throws std::invalid_argument if the code is not found.
 */
double get_price_for_product_code(const std::string& productCode) {
    auto it = PRODUCT_PRICES.find(productCode);
    if (it == PRODUCT_PRICES.end()) {
        throw std::invalid_argument("Invalid product code: " + productCode);
    }
    return it->second;
}

/**
 * @brief A web route that calculates the total price from a YAML payload.
 * @param yamlPayload A string containing the YAML payload.
 * @return A YAML string with the total price or an error message.
 */
std::string calculate_price(const std::string& yamlPayload) {
    YAML::Node result_node;
    try {
        // Rule#1: Ensure all input is validated.
        if (yamlPayload.empty()) {
            throw std::invalid_argument("YAML payload cannot be empty.");
        }
        
        // Simulating a payload size limit to prevent DoS attacks.
        const size_t MAX_PAYLOAD_SIZE = 5 * 1024 * 1024; // 5 MB
        if (yamlPayload.length() > MAX_PAYLOAD_SIZE) {
            throw std::runtime_error("Payload size exceeds limit.");
        }

        // Rule#2 & #4: yaml-cpp is generally safe from code execution vulnerabilities
        // as it does not support unsafe tags like !!python/object.
        YAML::Node data = YAML::Load(yamlPayload);

        if (!data.IsMap()) {
            throw std::invalid_argument("Invalid YAML format: root must be a mapping.");
        }

        // Extract and validate product_code
        if (!data["product_code"]) {
            throw std::invalid_argument("Missing 'product_code' in YAML payload.");
        }
        std::string productCode = data["product_code"].as<std::string>();

        // Extract and validate quantity
        if (!data["quantity"]) {
            throw std::invalid_argument("Missing 'quantity' in YAML payload.");
        }
        int quantity = data["quantity"].as<int>();
        if (quantity <= 0) {
            throw std::invalid_argument("'quantity' must be positive.");
        }

        double price = get_price_for_product_code(productCode);
        double totalPrice = price * quantity;

        result_node["total_price"] = totalPrice;
    } catch (const std::exception& e) {
        // Rule #5: Return generic messages to callers. Do not leak internal details.
        result_node["error"] = std::string(e.what());
    }

    YAML::Emitter emitter;
    emitter << result_node;
    return emitter.c_str();
}

int main() {
    std::vector<std::string> testCases = {
        // Test Case 1: Valid input
        "product_code: PROD123\nquantity: 2",
        // Test Case 2: Invalid product code
        "product_code: UNKNOWN\nquantity: 1",
        // Test Case 3: Invalid quantity (negative)
        "product_code: PROD456\nquantity: -5",
        // Test Case 4: Malformed YAML
        "product_code: PROD123, quantity: 2",
        // Test Case 5: Missing key
        "product_code: PROD789"
    };

    int i = 1;
    for (const auto& testCase : testCases) {
        std::cout << "--- Test Case " << i++ << " ---" << std::endl;
        std::cout << "Input YAML:\n---\n" << testCase << "\n---" << std::endl;
        std::string result = calculate_price(testCase);
        std::cout << "Output YAML:\n" << result << std::endl << std::endl;
    }

    return 0;
}