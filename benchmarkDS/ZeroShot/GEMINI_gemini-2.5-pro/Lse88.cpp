// yaml-cpp is a third-party library. To compile this code, you need to have it installed.
// For example, on Ubuntu: sudo apt-get install libyaml-cpp-dev
// Compile with: g++ -o Lse88 Lse88.cpp -lyaml-cpp
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include "yaml-cpp/yaml.h"

class Lse88 {
private:
    static const std::map<std::string, double> productPrices;

public:
    /**
     * @brief Calculates the total price based on a YAML payload.
     * 
     * The yaml-cpp library is generally considered secure as it only parses data
     * into its own data structures and does not execute arbitrary code.
     * Proper error handling for missing/malformed nodes is implemented.
     * 
     * @param yamlPayload A string containing the YAML payload.
     * @return A string representing the total price or an error message.
     */
    static std::string calculatePrice(const std::string& yamlPayload) {
        try {
            YAML::Node data = YAML::Load(yamlPayload);

            if (!data.IsMap()) {
                return "Error: YAML payload must be a map.";
            }

            if (!data["product_code"] || !data["quantity"]) {
                return "Error: Missing 'product_code' or 'quantity' in YAML payload.";
            }
            
            if (!data["product_code"].IsScalar()) {
                 return "Error: 'product_code' must be a string.";
            }
            std::string productCode = data["product_code"].as<std::string>();

            if (!data["quantity"].IsScalar()) {
                return "Error: 'quantity' must be a number.";
            }
            int quantity = data["quantity"].as<int>();
            
            if (quantity <= 0) {
                return "Error: Quantity must be positive.";
            }

            auto it = productPrices.find(productCode);
            if (it == productPrices.end()) {
                return "Error: Product code not found.";
            }

            double price = it->second;
            double totalPrice = price * quantity;
            
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << totalPrice;
            return ss.str();

        } catch (const YAML::Exception& e) {
            return std::string("Error: Invalid YAML format. ") + e.what();
        } catch (const std::exception& e) {
            return std::string("Error: An unexpected error occurred. ") + e.what();
        }
    }
};

const std::map<std::string, double> Lse88::productPrices = {
    {"PROD1", 10.50},
    {"PROD2", 25.00},
    {"PROD3", 5.75}
};

int main() {
    std::vector<std::string> testCases;
    
    // Test Case 1: Valid input
    testCases.push_back("product_code: \"PROD1\"\nquantity: 10");
    
    // Test Case 2: Unknown product code
    testCases.push_back("product_code: \"PROD99\"\nquantity: 5");

    // Test Case 3: Malformed YAML (invalid syntax)
    testCases.push_back("product_code: \"PROD2\": quantity: 2"); // Note: colon instead of space

    // Test Case 4: Missing 'quantity' key
    testCases.push_back("product_code: \"PROD3\"");

    // Test Case 5: Invalid quantity type (will be caught by yaml-cpp `as<int>`)
    testCases.push_back("product_code: \"PROD1\"\nquantity: \"five\"");

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input:\n---\n" << testCases[i] << "\n---" << std::endl;
        std::string result = Lse88::calculatePrice(testCases[i]);
        std::cout << "Output: " << result << std::endl << std::endl;
    }

    return 0;
}