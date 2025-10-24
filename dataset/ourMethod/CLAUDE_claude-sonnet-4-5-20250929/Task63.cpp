
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdint>
#include <memory>
#include <stdexcept>

// Base64 decoding implementation
class Base64 {
private:
    static const std::string base64_chars;
    
    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }
    
public:
    static std::vector<uint8_t> decode(const std::string& encoded_string) {
        // Validate input length
        size_t in_len = encoded_string.size();
        if (in_len == 0 || in_len % 4 != 0) {
            throw std::invalid_argument("Invalid base64 input length");
        }
        
        // Maximum output size check to prevent excessive memory allocation
        const size_t MAX_DECODE_SIZE = 10 * 1024 * 1024; // 10MB limit
        if (in_len / 4 * 3 > MAX_DECODE_SIZE) {
            throw std::invalid_argument("Input too large");
        }
        
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::vector<uint8_t> ret;
        ret.reserve(in_len / 4 * 3);
        
        while (in_len-- && encoded_string[in_] != '=') {
            // Validate character is valid base64
            if (!is_base64(encoded_string[in_])) {
                throw std::invalid_argument("Invalid base64 character");
            }
            
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++) {
                    char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));
                }
                
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                
                for (i = 0; i < 3; i++) {
                    ret.push_back(char_array_3[i]);
                }
                i = 0;
            }
        }
        
        if (i) {
            for (j = 0; j < i; j++) {
                char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));
            }
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            
            for (j = 0; j < i - 1; j++) {
                ret.push_back(char_array_3[j]);
            }
        }
        
        return ret;
    }
};

const std::string Base64::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Simple JSON parser for demonstration (handles basic string values)
class SimpleJSON {
public:
    static std::string parse(const std::vector<uint8_t>& data) {
        // Validate UTF-8 encoding
        if (!isValidUTF8(data)) {
            throw std::invalid_argument("Invalid UTF-8 data");
        }
        
        std::string json_str(data.begin(), data.end());
        
        // Validate JSON structure (basic validation)
        if (json_str.empty() || json_str.size() > 1024 * 1024) {
            throw std::invalid_argument("Invalid JSON data size");
        }
        
        // Check for balanced braces/brackets
        int brace_count = 0;
        int bracket_count = 0;
        bool in_string = false;
        
        for (size_t i = 0; i < json_str.size(); i++) {
            char c = json_str[i];
            
            if (c == '"' && (i == 0 || json_str[i-1] != '\\\\')) {\n                in_string = !in_string;\n            }\n            \n            if (!in_string) {\n                if (c == '{') brace_count++;\n                else if (c == '}') brace_count--;\n                else if (c == '[') bracket_count++;\n                else if (c == ']') bracket_count--;\n                \n                if (brace_count < 0 || bracket_count < 0) {\n                    throw std::invalid_argument("Malformed JSON");\n                }\n            }\n        }\n        \n        if (brace_count != 0 || bracket_count != 0) {\n            throw std::invalid_argument("Unbalanced JSON braces/brackets");\n        }\n        \n        return json_str;\n    }\n    \nprivate:\n    // Validate UTF-8 encoding\n    static bool isValidUTF8(const std::vector<uint8_t>& data) {\n        size_t i = 0;\n        while (i < data.size()) {\n            if (data[i] <= 0x7F) {\n                i++;\n            } else if ((data[i] & 0xE0) == 0xC0) {\n                if (i + 1 >= data.size() || (data[i+1] & 0xC0) != 0x80) return false;\n                i += 2;\n            } else if ((data[i] & 0xF0) == 0xE0) {\n                if (i + 2 >= data.size() || (data[i+1] & 0xC0) != 0x80 || (data[i+2] & 0xC0) != 0x80) return false;\n                i += 3;\n            } else if ((data[i] & 0xF8) == 0xF0) {\n                if (i + 3 >= data.size() || (data[i+1] & 0xC0) != 0x80 || (data[i+2] & 0xC0) != 0x80 || (data[i+3] & 0xC0) != 0x80) return false;\n                i += 4;\n            } else {\n                return false;\n            }\n        }\n        return true;\n    }\n};\n\n// Main request processor\nclass RequestProcessor {\npublic:\n    static std::string processRequest(const std::string& raw_data) {\n        // Step 1: Validate input is not empty and has reasonable size\n        if (raw_data.empty()) {\n            throw std::invalid_argument("Empty raw_data");\n        }\n        \n        const size_t MAX_INPUT_SIZE = 5 * 1024 * 1024; // 5MB limit\n        if (raw_data.size() > MAX_INPUT_SIZE) {\n            throw std::invalid_argument("Input data too large");\n        }\n        \n        // Step 2: Ensure UTF-8 encoding by validating the input string\n        for (size_t i = 0; i < raw_data.size(); i++) {\n            // Check for valid base64 characters (will be validated more in decode)\n            unsigned char c = static_cast<unsigned char>(raw_data[i]);\n            if (!(isalnum(c) || c == '+' || c == '/' || c == '=' || c == '\
' || c == '\\r' || c == ' ')) {\n                throw std::invalid_argument("Invalid character in raw_data");\n            }\n        }\n        \n        // Remove whitespace characters\n        std::string cleaned_data;\n        cleaned_data.reserve(raw_data.size());\n        for (char c : raw_data) {\n            if (c != '\
' && c != '\\r' && c != ' ') {
                cleaned_data += c;
            }
        }
        
        // Step 3: Decode base64 data
        std::vector<uint8_t> decoded_data = Base64::decode(cleaned_data);
        
        // Step 4: Deserialize (parse as JSON)
        std::string deserialized_data = SimpleJSON::parse(decoded_data);
        
        // Step 5: Return the deserialized data
        return deserialized_data;
    }
};

int main() {
    // Test case 1: Valid JSON object
    try {
        std::string test1 = "eyJuYW1lIjoiSm9obiIsImFnZSI6MzB9"; // {"name":"John","age":30}
        std::string result1 = RequestProcessor::processRequest(test1);
        std::cout << "Test 1 Result: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Valid JSON array
    try {
        std::string test2 = "WyJhcHBsZSIsImJhbmFuYSIsImNoZXJyeSJd"; // ["apple","banana","cherry"]
        std::string result2 = RequestProcessor::processRequest(test2);
        std::cout << "Test 2 Result: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Simple string value
    try {
        std::string test3 = "ImhlbGxvIHdvcmxkIg=="; // "hello world"
        std::string result3 = RequestProcessor::processRequest(test3);
        std::cout << "Test 3 Result: " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Empty input (should fail)
    try {
        std::string test4 = "";
        std::string result4 = RequestProcessor::processRequest(test4);
        std::cout << "Test 4 Result: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid base64 (should fail)
    try {
        std::string test5 = "Invalid!@#$%";
        std::string result5 = RequestProcessor::processRequest(test5);
        std::cout << "Test 5 Result: " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 Error: " << e.what() << std::endl;
    }
    
    return 0;
}
