
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <stdexcept>
#include <memory>

// Maximum payload size to prevent DoS attacks
const size_t MAX_PAYLOAD_SIZE = 10 * 1024; // 10KB
const size_t MAX_NAME_LENGTH = 100;

// Simple JSON parser for extracting 'name' field
class SimpleJSONParser {
private:
    std::string payload;
    
    // Finds the value of a key in simple JSON format
    std::string extractValue(const std::string& key) {
        // Pattern to match "key":"value" with proper escaping
        std::string pattern = "\\"" + key + "\\"\\\\s*:\\\\s*\\"([^\\"]*)\\"";
        std::regex keyRegex(pattern);
        std::smatch match;
        
        if (std::regex_search(payload, match, keyRegex) && match.size() > 1) {
            return match[1].str();
        }
        return "";
    }
    
public:
    explicit SimpleJSONParser(const std::string& json) : payload(json) {}
    
    bool hasKey(const std::string& key) {
        std::string pattern = "\\"" + key + "\\"\\\\s*:";
        std::regex keyRegex(pattern);
        return std::regex_search(payload, keyRegex);
    }
    
    std::string getValue(const std::string& key) {
        return extractValue(key);
    }
};

// Validates and extracts the 'name' field from JSON payload
std::string processPayload(const std::string& payload) {
    // Input validation: check payload is not empty
    if (payload.empty()) {
        return "Error: Empty payload";
    }
    
    // Input validation: check payload size to prevent DoS
    if (payload.length() > MAX_PAYLOAD_SIZE) {
        return "Error: Payload too large";
    }
    
    try {
        // Parse JSON
        SimpleJSONParser parser(payload);
        
        // Check if 'name' field exists
        if (parser.hasKey("name")) {
            std::string name = parser.getValue("name");
            
            // Validate name is not empty
            if (name.empty()) {
                return "Error: Name is empty";
            }
            
            // Input validation: check name length
            if (name.length() > MAX_NAME_LENGTH) {
                return "Error: Name too long";
            }
            
            // Input validation: sanitize by allowing only alphanumeric, space, and basic punctuation
            std::regex validChars("^[a-zA-Z0-9 ._-]*$");
            if (!std::regex_match(name, validChars)) {
                return "Error: Invalid characters in name";
            }
            
            // Return sanitized name content
            return name;
        } else {
            return "Error: Name field not found";
        }
    } catch (const std::exception& e) {
        // Log error internally, return generic message to user
        std::cerr << "Processing error: " << e.what() << std::endl;
        return "Error: Invalid JSON format";
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1 - Valid name:" << std::endl;
    std::cout << processPayload("{\\"name\\":\\"John_Doe\\"}") << std::endl;
    
    std::cout << "\\nTest Case 2 - Missing name field:" << std::endl;
    std::cout << processPayload("{\\"age\\":30}") << std::endl;
    
    std::cout << "\\nTest Case 3 - Invalid JSON:" << std::endl;
    std::cout << processPayload("{invalid json}") << std::endl;
    
    std::cout << "\\nTest Case 4 - Name with invalid characters:" << std::endl;
    std::cout << processPayload("{\\"name\\":\\"<script>alert('xss')</script>\\"}") << std::endl;
    
    std::cout << "\\nTest Case 5 - Empty payload:" << std::endl;
    std::cout << processPayload("") << std::endl;
    
    return 0;
}
