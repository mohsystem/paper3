
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <cstring>
#include <limits>

// Maximum payload size to prevent memory exhaustion (1KB)
const size_t MAX_PAYLOAD_SIZE = 1024;

// Maximum quantity to prevent integer overflow
const int MAX_QUANTITY = 1000000;

// Hardcoded price database (in production, use a secure database)
std::map<std::string, double> PRICE_DATABASE = {
    {"PROD001", 10.50},
    {"PROD002", 25.99},
    {"PROD003", 5.75},
    {"PROD004", 100.00},
    {"PROD005", 15.25}
};

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, last - first + 1);
}

/**
 * Returns the price for a given product code.
 * Uses input validation to prevent injection attacks.
 */
double get_price_for_product_code(const std::string& product_code) {
    // Input validation: check empty
    if (product_code.empty()) {
        throw std::invalid_argument("Product code cannot be empty");
    }
    
    std::string code = trim(product_code);
    
    // Input validation: check format (alphanumeric only, 1-20 chars)
    std::regex pattern("^[a-zA-Z0-9]{1,20}$");
    if (!std::regex_match(code, pattern)) {
        throw std::invalid_argument("Invalid product code format");
    }
    
    // Check if product exists
    auto it = PRICE_DATABASE.find(code);
    if (it == PRICE_DATABASE.end()) {
        throw std::invalid_argument("Product code not found");
    }
    
    return it->second;
}

// Simple YAML parser for key: value format (safe, no external libraries)
std::map<std::string, std::string> parse_simple_yaml(const std::string& yaml) {
    std::map<std::string, std::string> result;
    std::istringstream stream(yaml);
    std::string line;
    
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) continue;
        
        std::string key = trim(line.substr(0, colon_pos));
        std::string value = trim(line.substr(colon_pos + 1));
        
        // Remove quotes if present
        if (!value.empty() && (value.front() == '"' || value.front() == '\\'')) {
            value = value.substr(1, value.length() - 2);
        }
        
        result[key] = value;
    }
    
    return result;
}

// Create YAML response string
std::string create_yaml_response(const std::string& status, const std::string& key, const std::string& value) {
    std::ostringstream oss;
    oss << "status: " << status << "\\n";
    oss << key << ": " << value << "\\n";
    return oss.str();
}

/**
 * Calculates total price from YAML payload.
 * Uses simple parser to prevent arbitrary code execution.
 */
std::string calculate_price(const std::string& yaml_payload) {
    try {
        // Input validation: check size limits
        if (yaml_payload.length() > MAX_PAYLOAD_SIZE) {
            return create_yaml_response("error", "message", "Payload exceeds maximum size");
        }
        
        // Parse YAML safely (simple key-value parser, no code execution)
        std::map<std::string, std::string> data = parse_simple_yaml(yaml_payload);
        
        // Extract and validate product_code
        auto code_it = data.find("product_code");
        if (code_it == data.end() || code_it->second.empty()) {
            return create_yaml_response("error", "message", "Missing or invalid product_code");
        }
        std::string product_code = code_it->second;
        
        // Extract and validate quantity
        auto qty_it = data.find("quantity");
        if (qty_it == data.end()) {
            return create_yaml_response("error", "message", "Missing quantity");
        }
        
        int quantity = 0;
        try {
            // Use stoi with error checking
            size_t pos;
            quantity = std::stoi(qty_it->second, &pos);
            // Check if entire string was converted
            if (pos != qty_it->second.length()) {
                return create_yaml_response("error", "message", "Invalid quantity format");
            }
        } catch (const std::exception&) {
            return create_yaml_response("error", "message", "Invalid quantity format");
        }
        
        // Validate quantity range
        if (quantity <= 0 || quantity > MAX_QUANTITY) {
            return create_yaml_response("error", "message", "Quantity out of valid range");
        }
        
        // Get price for product code (with validation inside)
        double unit_price = get_price_for_product_code(product_code);
        
        // Calculate total with overflow check
        double total_price = unit_price * quantity;
        
        // Check for overflow
        if (std::isinf(total_price) || std::isnan(total_price)) {
            return create_yaml_response("error", "message", "Calculation overflow");
        }
        
        // Format price to 2 decimal places
        std::ostringstream price_stream;
        price_stream.precision(2);
        price_stream << std::fixed << total_price;
        
        return create_yaml_response("success", "total_price", price_stream.str());
        
    } catch (const std::invalid_argument&) {
        return create_yaml_response("error", "message", "Invalid input");
    } catch (const std::exception&) {
        return create_yaml_response("error", "message", "Processing error");
    }
}

int main() {
    std::cout << "Test Case 1: Valid input\\n";
    std::string payload1 = "product_code: PROD001\\nquantity: 5";
    std::cout << calculate_price(payload1) << "\\n";
    
    std::cout << "\\nTest Case 2: Different product\\n";
    std::string payload2 = "product_code: PROD003\\nquantity: 10";
    std::cout << calculate_price(payload2) << "\\n";
    
    std::cout << "\\nTest Case 3: Invalid product code\\n";
    std::string payload3 = "product_code: INVALID\\nquantity: 5";
    std::cout << calculate_price(payload3) << "\\n";
    
    std::cout << "\\nTest Case 4: Negative quantity\\n";
    std::string payload4 = "product_code: PROD002\\nquantity: -5";
    std::cout << calculate_price(payload4) << "\\n";
    
    std::cout << "\\nTest Case 5: Missing quantity\\n";
    std::string payload5 = "product_code: PROD001";
    std::cout << calculate_price(payload5) << "\\n";
    
    return 0;
}
