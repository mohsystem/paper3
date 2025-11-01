#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
// To compile this code, you need the yaml-cpp library.
// Example compilation command:
// g++ -std=c++17 your_file.cpp -o your_app -I/path/to/yaml-cpp/include -L/path/to/yaml-cpp/build -lyaml-cpp
#include "yaml-cpp/yaml.h"

class Lse90 {
private:
    static const std::map<std::string, double> productPrices;

public:
    /**
     * @brief Returns the price for a given product code.
     * @param productCode The code of the product.
     * @return The price of the product, or -1.0 if not found.
     */
    static double get_price_for_product_code(const std::string& productCode) {
        auto it = productPrices.find(productCode);
        if (it != productPrices.end()) {
            return it->second;
        }
        return -1.0;
    }

    /**
     * @brief Simulates a web route that parses a YAML payload, calculates the total price,
     * and returns the result as a YAML payload.
     * yaml-cpp is generally safe from arbitrary code execution as it parses into
     * its own data structures, not arbitrary C++ objects.
     * @param yamlPayload A string containing the YAML request.
     * @return A YAML string with the total price or an error message.
     */
    static std::string calculate_price(const std::string& yamlPayload) {
        YAML::Emitter emitter;
        emitter << YAML::BeginMap;

        try {
            YAML::Node data = YAML::Load(yamlPayload);
            if (!data.IsMap() || !data["product_code"] || !data["quantity"]) {
                emitter << YAML::Key << "error" << YAML::Value << "Invalid payload: 'product_code' and 'quantity' are required.";
            } else {
                std::string productCode = data["product_code"].as<std::string>();
                int quantity = data["quantity"].as<int>();

                if (quantity <= 0) {
                    emitter << YAML::Key << "error" << YAML::Value << "Quantity must be positive.";
                } else {
                    double price = get_price_for_product_code(productCode);
                    if (price < 0) {
                        emitter << YAML::Key << "error" << YAML::Value << "Product code not found.";
                    } else {
                        double totalPrice = price * quantity;
                        emitter << YAML::Key << "total_price" << YAML::Value << totalPrice;
                    }
                }
            }
        } catch (const YAML::Exception& e) {
            emitter << YAML::Key << "error" << YAML::Value << "Failed to parse YAML payload or invalid data types.";
        } catch (const std::exception& e) {
            emitter << YAML::Key << "error" << YAML::Value << "An unexpected error occurred.";
        }
        
        emitter << YAML::EndMap;
        return emitter.c_str();
    }
};

// Initialize static map
const std::map<std::string, double> Lse90::productPrices = {
    {"PROD001", 10.50},
    {"PROD002", 25.00},
    {"PROD003", 5.75},
    {"PROD004", 150.00}
};

int main() {
    std::vector<std::string> testCases = {
        // Test Case 1: Valid request
        "product_code: PROD001\nquantity: 10",
        // Test Case 2: Another valid request
        "product_code: PROD004\nquantity: 2",
        // Test Case 3: Product code not found
        "product_code: PROD999\nquantity: 5",
        // Test Case 4: Invalid payload (missing quantity)
        "product_code: PROD002",
        // Test Case 5: Malformed YAML / Type mismatch
        "product_code: PROD003\nquantity: [1, 2, 3]"
    };

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (int i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input:\n---\n" << testCases[i] << "\n---" << std::endl;
        std::string result = Lse90::calculate_price(testCases[i]);
        std::cout << "Output:\n" << result << std::endl << std::endl;
    }

    return 0;
}