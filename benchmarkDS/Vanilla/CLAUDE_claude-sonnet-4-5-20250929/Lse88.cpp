
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>

// Simple YAML parser for basic key-value pairs
std::map<std::string, std::string> parseYAML(const std::string& yaml) {
    std::map<std::string, std::string> result;
    std::istringstream stream(yaml);
    std::string line;
    
    while (std::getline(stream, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \\t"));
            key.erase(key.find_last_not_of(" \\t") + 1);
            value.erase(0, value.find_first_not_of(" \\t"));
            value.erase(value.find_last_not_of(" \\t") + 1);
            
            result[key] = value;
        }
    }
    return result;
}

class Lse88 {
private:
    static std::map<std::string, double> productPrices;
    
public:
    static void initPrices() {
        productPrices["PROD001"] = 10.99;
        productPrices["PROD002"] = 25.50;
        productPrices["PROD003"] = 5.75;
        productPrices["PROD004"] = 100.00;
        productPrices["PROD005"] = 15.25;
    }
    
    static std::string calculatePrice(const std::string& yamlPayload) {
        try {
            std::map<std::string, std::string> data = parseYAML(yamlPayload);
            
            std::string productCode = data["product_code"];
            int quantity = std::stoi(data["quantity"]);
            
            if (productPrices.find(productCode) == productPrices.end()) {
                return "Error: Product not found";
            }
            
            double price = productPrices[productCode];
            double totalPrice = price * quantity;
            
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << totalPrice;
            return oss.str();
        } catch (const std::exception& e) {
            return "Error: Invalid YAML payload - " + std::string(e.what());
        }
    }
};

std::map<std::string, double> Lse88::productPrices;

int main() {
    Lse88::initPrices();
    
    // Test case 1: Valid product with quantity 2
    std::string test1 = "product_code: PROD001\\nquantity: 2";
    std::cout << "Test 1: " << Lse88::calculatePrice(test1) << std::endl;
    
    // Test case 2: Valid product with quantity 5
    std::string test2 = "product_code: PROD002\\nquantity: 5";
    std::cout << "Test 2: " << Lse88::calculatePrice(test2) << std::endl;
    
    // Test case 3: Valid product with quantity 1
    std::string test3 = "product_code: PROD003\\nquantity: 1";
    std::cout << "Test 3: " << Lse88::calculatePrice(test3) << std::endl;
    
    // Test case 4: Invalid product code
    std::string test4 = "product_code: PROD999\\nquantity: 3";
    std::cout << "Test 4: " << Lse88::calculatePrice(test4) << std::endl;
    
    // Test case 5: Valid product with large quantity
    std::string test5 = "product_code: PROD004\\nquantity: 10";
    std::cout << "Test 5: " << Lse88::calculatePrice(test5) << std::endl;
    
    return 0;
}
