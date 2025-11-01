
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

class Lse88 {
private:
    static std::map<std::string, double> PRODUCT_PRICES;
    
    static std::map<std::string, std::string> parseYAML(const std::string& yaml_payload) {
        std::map<std::string, std::string> result;
        std::istringstream stream(yaml_payload);
        std::string line;
        
        while (std::getline(stream, line)) {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                
                // Trim whitespace
                key.erase(0, key.find_first_not_of(" \\t\\r\\n"));
                key.erase(key.find_last_not_of(" \\t\\r\\n") + 1);
                value.erase(0, value.find_first_not_of(" \\t\\r\\n"));
                value.erase(value.find_last_not_of(" \\t\\r\\n") + 1);
                
                result[key] = value;
            }
        }
        
        return result;
    }
    
public:
    static std::string calculatePrice(const std::string& yaml_payload) {
        try {
            if (yaml_payload.empty()) {
                return "Error: Empty payload";
            }
            
            std::map<std::string, std::string> data = parseYAML(yaml_payload);
            
            if (data.find("product_code") == data.end() || data.find("quantity") == data.end()) {
                return "Error: Missing product_code or quantity";
            }
            
            std::string product_code = data["product_code"];
            int quantity = std::stoi(data["quantity"]);
            
            if (quantity <= 0) {
                return "Error: Quantity must be positive";
            }
            
            if (PRODUCT_PRICES.find(product_code) == PRODUCT_PRICES.end()) {
                return "Error: Product not found";
            }
            
            double price = PRODUCT_PRICES[product_code];
            double total_price = price * quantity;
            
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << total_price;
            return oss.str();
            
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
};

std::map<std::string, double> Lse88::PRODUCT_PRICES = {
    {"PROD001", 10.50},
    {"PROD002", 25.99},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};

int main() {
    // Test case 1: Valid product and quantity
    std::string test1 = "product_code: PROD001\\nquantity: 3";
    std::cout << "Test 1: " << Lse88::calculatePrice(test1) << std::endl;
    
    // Test case 2: Different product
    std::string test2 = "product_code: PROD002\\nquantity: 2";
    std::cout << "Test 2: " << Lse88::calculatePrice(test2) << std::endl;
    
    // Test case 3: Invalid product code
    std::string test3 = "product_code: INVALID\\nquantity: 5";
    std::cout << "Test 3: " << Lse88::calculatePrice(test3) << std::endl;
    
    // Test case 4: Missing quantity
    std::string test4 = "product_code: PROD003";
    std::cout << "Test 4: " << Lse88::calculatePrice(test4) << std::endl;
    
    // Test case 5: Zero quantity
    std::string test5 = "product_code: PROD004\\nquantity: 0";
    std::cout << "Test 5: " << Lse88::calculatePrice(test5) << std::endl;
    
    return 0;
}
