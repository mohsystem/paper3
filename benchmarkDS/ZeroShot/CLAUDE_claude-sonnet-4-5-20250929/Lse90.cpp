
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <yaml-cpp/yaml.h>

class Lse90 {
private:
    static std::map<std::string, double> productPrices;
    
    static std::string createErrorResponse(const std::string& message) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "error" << YAML::Value << message;
        out << YAML::EndMap;
        return out.c_str();
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, last - first + 1);
    }
    
    static std::string toUpper(std::string str) {
        for (char& c : str) {
            c = toupper(c);
        }
        return str;
    }

public:
    static void initPrices() {
        if (productPrices.empty()) {
            productPrices["PROD001"] = 10.99;
            productPrices["PROD002"] = 25.50;
            productPrices["PROD003"] = 15.75;
            productPrices["PROD004"] = 8.99;
            productPrices["PROD005"] = 100.00;
        }
    }
    
    static double getPriceForProductCode(const std::string& productCode) {
        initPrices();
        std::string code = toUpper(trim(productCode));
        auto it = productPrices.find(code);
        if (it != productPrices.end()) {
            return it->second;
        }
        return -1.0;
    }
    
    static std::string calculatePrice(const std::string& yamlPayload) {
        if (yamlPayload.empty()) {
            return createErrorResponse("Invalid payload");
        }
        
        try {
            YAML::Node data = YAML::Load(yamlPayload);
            
            if (!data["product_code"] || !data["quantity"]) {
                return createErrorResponse("Missing required fields");
            }
            
            std::string productCode = data["product_code"].as<std::string>();
            int quantity;
            
            try {
                quantity = data["quantity"].as<int>();
                if (quantity <= 0) {
                    return createErrorResponse("Quantity must be positive");
                }
            } catch (...) {
                return createErrorResponse("Invalid quantity format");
            }
            
            double price = getPriceForProductCode(productCode);
            if (price < 0) {
                return createErrorResponse("Product not found");
            }
            
            double totalPrice = price * quantity;
            
            YAML::Emitter out;
            out << YAML::BeginMap;
            out << YAML::Key << "product_code" << YAML::Value << productCode;
            out << YAML::Key << "quantity" << YAML::Value << quantity;
            out << YAML::Key << "unit_price" << YAML::Value << price;
            out << YAML::Key << "total_price" << YAML::Value << totalPrice;
            out << YAML::EndMap;
            
            return out.c_str();
            
        } catch (const YAML::Exception& e) {
            return createErrorResponse("Invalid YAML format");
        } catch (...) {
            return createErrorResponse("Error processing request");
        }
    }
};

std::map<std::string, double> Lse90::productPrices;

int main() {
    std::cout << "Test Case 1: Valid request\\n";
    std::string result1 = Lse90::calculatePrice("product_code: PROD001\\nquantity: 5");
    std::cout << result1 << "\\n\\n";
    
    std::cout << "Test Case 2: Different product\\n";
    std::string result2 = Lse90::calculatePrice("product_code: PROD003\\nquantity: 3");
    std::cout << result2 << "\\n\\n";
    
    std::cout << "Test Case 3: Invalid product code\\n";
    std::string result3 = Lse90::calculatePrice("product_code: INVALID\\nquantity: 2");
    std::cout << result3 << "\\n\\n";
    
    std::cout << "Test Case 4: Missing fields\\n";
    std::string result4 = Lse90::calculatePrice("product_code: PROD001");
    std::cout << result4 << "\\n\\n";
    
    std::cout << "Test Case 5: Negative quantity\\n";
    std::string result5 = Lse90::calculatePrice("product_code: PROD002\\nquantity: -5");
    std::cout << result5 << "\\n";
    
    return 0;
}
