
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <cstring>

// Maximum YAML payload size to prevent DoS (10KB)
const size_t MAX_YAML_SIZE = 10240;

// Maximum quantity to prevent overflow
const int MAX_QUANTITY = 10000;
const int MIN_QUANTITY = 1;

// Product prices stored as cents to avoid floating point issues
static std::map<std::string, int> PRODUCT_PRICES_CENTS;

// Initialize product catalog
void initializePrices() {
    PRODUCT_PRICES_CENTS["PROD-001"] = 1050;  // $10.50
    PRODUCT_PRICES_CENTS["PROD-002"] = 2500;  // $25.00
    PRODUCT_PRICES_CENTS["PROD-003"] = 599;   // $5.99
    PRODUCT_PRICES_CENTS["PROD-004"] = 10000; // $100.00
    PRODUCT_PRICES_CENTS["PROD-005"] = 1575;  // $15.75
}

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, last - first + 1);
}

// Validate product code format (alphanumeric and hyphens only)
bool isValidProductCode(const std::string& code) {
    if (code.empty() || code.length() > 50) return false;
    std::regex pattern("^[a-zA-Z0-9-]+$");
    return std::regex_match(code, pattern);
}

// Simple YAML parser for key-value pairs (secure implementation)
// Does not handle complex YAML features or external entities
bool parseSimpleYaml(const std::string& yaml, std::string& productCode, int& quantity) {
    std::istringstream stream(yaml);
    std::string line;
    bool foundProduct = false;
    bool foundQuantity = false;
    
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty()) continue;
        
        // Find colon separator
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) continue;
        
        std::string key = trim(line.substr(0, colonPos));
        std::string value = trim(line.substr(colonPos + 1));
        
        if (key == "product_code") {
            productCode = value;
            foundProduct = true;
        } else if (key == "quantity") {
            try {
                // Check for integer overflow and validate conversion
                size_t pos;
                long long temp = std::stoll(value, &pos);
                // Ensure entire string was parsed and value is in valid range
                if (pos != value.length()) return false;
                if (temp < INT_MIN || temp > INT_MAX) return false;
                quantity = static_cast<int>(temp);
                foundQuantity = true;
            } catch (...) {
                return false;
            }
        }
    }
    
    return foundProduct && foundQuantity;
}

/**
 * Calculates price based on YAML payload.
 * Security measures:
 * - Input size validation to prevent DoS
 * - Safe YAML parsing without entity expansion or external references
 * - Type validation for all extracted values
 * - Range validation for quantity
 * - Whitelist validation for product codes
 * - Overflow prevention using checked arithmetic
 * - Generic error messages to prevent information leakage
 */
std::string calculatePrice(const std::string& yamlPayload) {
    try {
        // Validate input is not empty
        if (yamlPayload.empty()) {
            return "Error: Invalid request";
        }
        
        // Validate payload size to prevent DoS attacks
        if (yamlPayload.length() > MAX_YAML_SIZE) {
            return "Error: Request too large";
        }
        
        std::string productCode;
        int quantity = 0;
        
        // Parse YAML safely - simple parser without external entity support
        if (!parseSimpleYaml(yamlPayload, productCode, quantity)) {
            return "Error: Invalid format";
        }
        
        // Validate product code format against whitelist pattern
        if (!isValidProductCode(productCode)) {
            return "Error: Invalid product code format";
        }
        
        // Validate quantity range
        if (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY) {
            return "Error: Quantity out of range";
        }
        
        // Check if product exists in catalog
        if (PRODUCT_PRICES_CENTS.find(productCode) == PRODUCT_PRICES_CENTS.end()) {
            return "Error: Product not found";
        }
        
        // Get unit price in cents
        int unitPriceCents = PRODUCT_PRICES_CENTS[productCode];
        
        // Check for multiplication overflow before calculating
        if (quantity > 0 && unitPriceCents > INT_MAX / quantity) {
            return "Error: Calculation overflow";
        }
        
        // Calculate total price in cents
        int totalCents = unitPriceCents * quantity;
        
        // Convert to dollars with 2 decimal places
        int dollars = totalCents / 100;
        int cents = totalCents % 100;
        
        std::ostringstream result;
        result << dollars << ".";
        if (cents < 10) result << "0";
        result << cents;
        
        return result.str();
        
    } catch (...) {
        // Generic error - log details securely but don't expose to user
        return "Error: Processing failed";
    }
}

int main() {
    // Initialize product prices
    initializePrices();
    
    std::cout << "Test Case 1 - Valid request:" << std::endl;
    std::string test1 = "product_code: PROD-001\\nquantity: 5";
    std::cout << "Input: product_code: PROD-001 | quantity: 5" << std::endl;
    std::cout << "Output: " << calculatePrice(test1) << std::endl << std::endl;
    
    std::cout << "Test Case 2 - Different product:" << std::endl;
    std::string test2 = "product_code: PROD-003\\nquantity: 10";
    std::cout << "Input: product_code: PROD-003 | quantity: 10" << std::endl;
    std::cout << "Output: " << calculatePrice(test2) << std::endl << std::endl;
    
    std::cout << "Test Case 3 - Invalid product code:" << std::endl;
    std::string test3 = "product_code: INVALID\\nquantity: 2";
    std::cout << "Input: product_code: INVALID | quantity: 2" << std::endl;
    std::cout << "Output: " << calculatePrice(test3) << std::endl << std::endl;
    
    std::cout << "Test Case 4 - Quantity out of range:" << std::endl;
    std::string test4 = "product_code: PROD-002\\nquantity: 20000";
    std::cout << "Input: product_code: PROD-002 | quantity: 20000" << std::endl;
    std::cout << "Output: " << calculatePrice(test4) << std::endl << std::endl;
    
    std::cout << "Test Case 5 - Missing field:" << std::endl;
    std::string test5 = "product_code: PROD-001";
    std::cout << "Input: product_code: PROD-001" << std::endl;
    std::cout << "Output: " << calculatePrice(test5) << std::endl;
    
    return 0;
}
