
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

// Secure deserialization class for structured data
// Uses a simple whitelist-based text format to avoid insecure binary deserialization
// Format: TYPE:LENGTH:VALUE where TYPE is single char (S=string, I=int, B=bool)
class SecureDeserializer {
private:
    // Maximum allowed string length to prevent resource exhaustion
    static const size_t MAX_STRING_LENGTH = 10000;
    // Maximum allowed field count to prevent DoS
    static const size_t MAX_FIELD_COUNT = 100;

    // Validate that input contains only safe characters for format
    static bool isValidFormatChar(char c) {
        return (c >= '0' && c <= '9') || c == ':' || c == 'S' || c == 'I' || 
               c == 'B' || c == '|' || c == '-' || c == ' ' || 
               (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    // Safe integer parsing with overflow checks
    static bool safeParseInt(const std::string& str, int64_t& result) {
        if (str.empty() || str.length() > 20) {
            return false; // Prevent overflow and empty input
        }
        
        try {
            size_t pos = 0;
            result = std::stoll(str, &pos);
            return pos == str.length(); // Ensure entire string was parsed
        } catch (...) {
            return false;
        }
    }

public:
    struct DeserializedData {
        std::string type;
        std::string value;
    };

    // Deserialize user-supplied data with strict validation
    // Input format: TYPE:LENGTH:VALUE|TYPE:LENGTH:VALUE|...
    // Example: S:5:Hello|I:3:123|B:4:true
    static std::vector<DeserializedData> deserialize(const std::string& input) {
        std::vector<DeserializedData> result;
        
        // Validate input is not empty
        if (input.empty()) {
            throw std::invalid_argument("Input cannot be empty");
        }
        
        // Validate input length to prevent DoS
        if (input.length() > MAX_STRING_LENGTH * 10) {
            throw std::invalid_argument("Input exceeds maximum allowed length");
        }
        
        // Split by field separator
        std::istringstream stream(input);
        std::string field;
        size_t fieldCount = 0;
        
        while (std::getline(stream, field, '|')) {
            // Prevent too many fields (DoS protection)
            if (++fieldCount > MAX_FIELD_COUNT) {
                throw std::invalid_argument("Too many fields in input");
            }
            
            // Parse field format: TYPE:LENGTH:VALUE
            size_t firstColon = field.find(':');
            size_t secondColon = field.find(':', firstColon + 1);
            
            if (firstColon == std::string::npos || secondColon == std::string::npos) {
                throw std::invalid_argument("Invalid field format - missing colons");
            }
            
            std::string typeStr = field.substr(0, firstColon);
            std::string lengthStr = field.substr(firstColon + 1, secondColon - firstColon - 1);
            std::string valueStr = field.substr(secondColon + 1);
            
            // Validate type is single character and whitelisted
            if (typeStr.length() != 1 || (typeStr[0] != 'S' && typeStr[0] != 'I' && typeStr[0] != 'B')) {
                throw std::invalid_argument("Invalid type - must be S, I, or B");
            }
            
            // Validate and parse length
            int64_t declaredLength = 0;
            if (!safeParseInt(lengthStr, declaredLength) || declaredLength < 0) {
                throw std::invalid_argument("Invalid length value");
            }
            
            // Validate declared length matches actual value length
            if (static_cast<size_t>(declaredLength) != valueStr.length()) {
                throw std::invalid_argument("Declared length does not match actual value length");
            }
            
            // Validate length constraints
            if (static_cast<size_t>(declaredLength) > MAX_STRING_LENGTH) {
                throw std::invalid_argument("Value exceeds maximum allowed length");
            }
            
            // Type-specific validation
            if (typeStr[0] == 'I') {
                // Validate integer value
                int64_t intValue = 0;
                if (!safeParseInt(valueStr, intValue)) {
                    throw std::invalid_argument("Invalid integer value");
                }
            } else if (typeStr[0] == 'B') {
                // Validate boolean value
                if (valueStr != "true" && valueStr != "false") {
                    throw std::invalid_argument("Invalid boolean value - must be 'true' or 'false'");
                }
            }
            // String type 'S' accepts any printable characters
            
            DeserializedData data;
            data.type = typeStr;
            data.value = valueStr;
            result.push_back(data);
        }
        
        return result;
    }
};

int main() {
    // Test case 1: Valid string field
    try {
        std::string input1 = "S:5:Hello";
        auto result1 = SecureDeserializer::deserialize(input1);
        std::cout << "Test 1 - String: " << result1[0].value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Valid integer field
    try {
        std::string input2 = "I:3:123";
        auto result2 = SecureDeserializer::deserialize(input2);
        std::cout << "Test 2 - Integer: " << result2[0].value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Multiple fields
    try {
        std::string input3 = "S:5:World|I:3:456|B:4:true";
        auto result3 = SecureDeserializer::deserialize(input3);
        std::cout << "Test 3 - Multiple fields: " << result3[0].value << ", " 
                  << result3[1].value << ", " << result3[2].value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Invalid length mismatch (should fail)
    try {
        std::string input4 = "S:3:Hello";
        auto result4 = SecureDeserializer::deserialize(input4);
        std::cout << "Test 4 - Should have failed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Correctly rejected: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid type (should fail)
    try {
        std::string input5 = "X:5:Hello";
        auto result5 = SecureDeserializer::deserialize(input5);
        std::cout << "Test 5 - Should have failed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Correctly rejected: " << e.what() << std::endl;
    }
    
    return 0;
}
