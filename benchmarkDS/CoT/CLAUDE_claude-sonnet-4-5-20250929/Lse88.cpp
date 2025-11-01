
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

class Lse88 {
private:
    static std::map<std::string, double> productPrices;
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }
    
    static bool parseSimpleYAML(const std::string& yaml, std::string& productCode, int& quantity) {
        std::istringstream stream(yaml);
        std::string line;
        bool hasProductCode = false;
        bool hasQuantity = false;
        
        while (std::getline(stream, line)) {
            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;
            
            std::string key = trim(line.substr(0, colonPos));
            std::string value = trim(line.substr(colonPos + 1));
            
            if (key == "product_code") {
                productCode = value;
                hasProductCode = true;
            } else if (key == "quantity") {
                try {
                    quantity = std::stoi(value);
                    hasQuantity = true;
                } catch (...) {
                    return false;
                }
            }
        }
        
        return hasProductCode && hasQuantity;
    }
    
public:
    static void initializePrices() {
        productPrices["PROD001"] = 10.50;
        productPrices["PROD002"] = 25.00;
        productPrices["PROD003"] = 15.75;
        productPrices["PROD004"] = 30.00;
        productPrices["PROD005"] = 5.99;
    }
    
    static std::string calculatePrice(const std::string& yamlPayload) {
        std::string productCode;
        int quantity = 0;
        
        // Parse YAML safely (simple parser to avoid vulnerabilities)
        if (!parseSimpleYAML(yamlPayload, productCode, quantity)) {
            return "Error: Invalid YAML format or missing fields";
        }
        
        // Validate product code
        if (productPrices.find(productCode) == productPrices.end()) {
            return "Error: Invalid product code";
        }
        
        // Validate quantity
        if (quantity <= 0 || quantity > 10000) {
            return "Error: Quantity must be between 1 and 10000";
        }
        
        // Calculate total price
        double price = productPrices[productCode];
        double totalPrice = price * quantity;
        
        std::ostringstream result;
        result.precision(2);
        result << std::fixed << totalPrice;
        
        return result.str();
    }
};

std::map<std::string, double> Lse88::productPrices;

int main() {
    Lse88::initializePrices();
    
    // Test case 1: Valid input
    std::string test1 = "product_code: PROD001\\nquantity: 5";
    std::cout << "Test 1: " << Lse88::calculatePrice(test1) << std::endl;
    
    // Test case 2: Different product
    std::string test2 = "product_code: PROD003\\nquantity: 10";
    std::cout << "Test 2: " << Lse88::calculatePrice(test2) << std::endl;
    
    // Test case 3: Invalid product code
    std::string test3 = "product_code: INVALID\\nquantity: 3";
    std::cout << "Test 3: " << Lse88::calculatePrice(test3) << std::endl;
    
    // Test case 4: Invalid quantity
    std::string test4 = "product_code: PROD002\\nquantity: -5";
    std::cout << "Test 4: " << Lse88::calculatePrice(test4) << std::endl;
    
    // Test case 5: Missing fields
    std::string test5 = "product_code: PROD001";
    std::cout << "Test 5: " << Lse88::calculatePrice(test5) << std::endl;
    
    return 0;
}
