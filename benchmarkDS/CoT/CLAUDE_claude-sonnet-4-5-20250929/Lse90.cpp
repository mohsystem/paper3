
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

class Lse90 {
private:
    static const int MAX_QUANTITY = 10000;
    static const int MAX_YAML_SIZE = 1024;
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, last - first + 1);
    }
    
    static std::string toUpper(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    
    static std::map<std::string, std::string> parseYaml(const std::string& yaml) {
        std::map<std::string, std::string> result;
        std::istringstream stream(yaml);
        std::string line;
        
        while (std::getline(stream, line)) {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                result[key] = value;
            }
        }
        return result;
    }
    
    static std::string createErrorYaml(const std::string& message) {
        return "error: " + message + "\\n";
    }

public:
    static double getPriceForProductCode(const std::string& productCode) {
        if (productCode.empty()) {
            return 0.0;
        }
        
        std::map<std::string, double> prices;
        prices["PROD001"] = 10.99;
        prices["PROD002"] = 25.50;
        prices["PROD003"] = 15.75;
        prices["PROD004"] = 8.99;
        prices["PROD005"] = 99.99;
        
        std::string upperCode = toUpper(trim(productCode));
        
        if (prices.find(upperCode) != prices.end()) {
            return prices[upperCode];
        }
        return 0.0;
    }
    
    static std::string calculatePrice(const std::string& yamlPayload) {
        try {
            if (yamlPayload.empty() || yamlPayload.length() > MAX_YAML_SIZE) {
                return createErrorYaml("Invalid payload size");
            }
            
            std::map<std::string, std::string> data = parseYaml(yamlPayload);
            
            if (data.find("product_code") == data.end() || 
                data.find("quantity") == data.end()) {
                return createErrorYaml("Missing required fields");
            }
            
            std::string productCode = data["product_code"];
            int quantity;
            
            try {
                quantity = std::stoi(data["quantity"]);
                if (quantity <= 0 || quantity > MAX_QUANTITY) {
                    return createErrorYaml("Invalid quantity");
                }
            } catch (...) {
                return createErrorYaml("Invalid quantity format");
            }
            
            double unitPrice = getPriceForProductCode(productCode);
            if (unitPrice == 0.0) {
                return createErrorYaml("Product not found");
            }
            
            double totalPrice = unitPrice * quantity;
            
            std::ostringstream result;
            result << "product_code: " << productCode << "\\n";
            result << "quantity: " << quantity << "\\n";
            result << "unit_price: " << unitPrice << "\\n";
            result << "total_price: " << totalPrice << "\\n";
            
            return result.str();
            
        } catch (...) {
            return createErrorYaml("Processing error");
        }
    }
};

int main() {
    std::cout << "Test Case 1: Valid product\\n";
    std::string yaml1 = "product_code: PROD001\\nquantity: 5";
    std::cout << Lse90::calculatePrice(yaml1) << "\\n";
    
    std::cout << "\\nTest Case 2: Different product\\n";
    std::string yaml2 = "product_code: PROD003\\nquantity: 10";
    std::cout << Lse90::calculatePrice(yaml2) << "\\n";
    
    std::cout << "\\nTest Case 3: Invalid product code\\n";
    std::string yaml3 = "product_code: INVALID\\nquantity: 2";
    std::cout << Lse90::calculatePrice(yaml3) << "\\n";
    
    std::cout << "\\nTest Case 4: Invalid quantity\\n";
    std::string yaml4 = "product_code: PROD002\\nquantity: -5";
    std::cout << Lse90::calculatePrice(yaml4) << "\\n";
    
    std::cout << "\\nTest Case 5: Missing fields\\n";
    std::string yaml5 = "product_code: PROD001";
    std::cout << Lse90::calculatePrice(yaml5) << "\\n";
    
    return 0;
}
