
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <memory>
#include <sstream>
#include <algorithm>

// Simple Base64 implementation
class Base64 {
private:
    static const std::string base64_chars;
    
public:
    static std::string decode(const std::string& encoded_string) {
        int in_len = encoded_string.size();
        int i = 0, j = 0, in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;
        
        while (in_len-- && (encoded_string[in_] != '=')) {
            if (!is_base64(encoded_string[in_])) {
                throw std::invalid_argument("Invalid Base64 character");
            }
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                
                for (i = 0; i < 3; i++)
                    ret += char_array_3[i];
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 4; j++)
                char_array_4[j] = 0;
            
            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            
            for (j = 0; j < i - 1; j++)
                ret += char_array_3[j];
        }
        
        return ret;
    }
    
    static std::string encode(const std::string& input) {
        std::string ret;
        int i = 0, j = 0;
        unsigned char char_array_3[3], char_array_4[4];
        int in_len = input.size();
        const char* bytes_to_encode = input.c_str();
        
        while (in_len--) {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while(i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
private:
    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }
};

const std::string Base64::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// Simple JSON parser for demonstration (validates basic structure)
class SimpleJSON {
public:
    static bool isValidJSON(const std::string& jsonStr) {
        std::string trimmed = trim(jsonStr);
        if (trimmed.empty()) return false;
        
        // Basic validation for JSON object or array
        if ((trimmed.front() == '{' && trimmed.back() == '}') ||
            (trimmed.front() == '[' && trimmed.back() == ']')) {
            return true;
        }
        return false;
    }
    
private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }
};

class Task63 {
private:
    static const size_t MAX_DATA_SIZE = 1024 * 1024; // 1MB limit
    
public:
    static std::string processRequest(const std::string& rawData) {
        // Validate input
        if (rawData.empty()) {
            throw std::invalid_argument("Raw data cannot be null or empty");
        }
        
        // Validate data size to prevent DoS attacks
        if (rawData.length() > MAX_DATA_SIZE) {
            throw std::invalid_argument("Data exceeds maximum allowed size");
        }
        
        try {
            // Ensure UTF-8 encoding (already handled in C++ string)
            std::string utf8String = rawData;
            
            // Decode Base64 with validation
            std::string decodedString = Base64::decode(utf8String);
            
            // Validate decoded size
            if (decodedString.length() > MAX_DATA_SIZE) {
                throw std::invalid_argument("Decoded data exceeds maximum allowed size");
            }
            
            // Validate JSON structure
            if (!SimpleJSON::isValidJSON(decodedString)) {
                throw std::invalid_argument("Invalid JSON format");
            }
            
            // Return deserialized data (as string for simplicity)
            return decodedString;
            
        } catch (const std::invalid_argument& e) {
            std::cerr << "Decoding error: " << e.what() << std::endl;
            throw std::runtime_error("Invalid encoded data");
        } catch (const std::exception& e) {
            std::cerr << "Processing error: " << e.what() << std::endl;
            throw std::runtime_error("Error processing request");
        }
    }
};

int main() {
    std::cout << "Testing Task63 - Secure Request Processing\\n" << std::endl;
    
    // Test Case 1: Valid JSON object
    try {
        std::string testData1 = Base64::encode("{\\"name\\":\\"John\\",\\"age\\":30}");
        std::string result1 = Task63::processRequest(testData1);
        std::cout << "Test 1 - Valid JSON: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 Failed: " << e.what() << std::endl;
    }
    
    // Test Case 2: Valid JSON array
    try {
        std::string testData2 = Base64::encode("[\\"item1\\",\\"item2\\",\\"item3\\"]");
        std::string result2 = Task63::processRequest(testData2);
        std::cout << "Test 2 - Valid JSON Array: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 Failed: " << e.what() << std::endl;
    }
    
    // Test Case 3: Nested JSON
    try {
        std::string testData3 = Base64::encode("{\\"user\\":{\\"id\\":123,\\"status\\":\\"active\\"}}");
        std::string result3 = Task63::processRequest(testData3);
        std::cout << "Test 3 - Nested JSON: " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 Failed: " << e.what() << std::endl;
    }
    
    // Test Case 4: Invalid Base64
    try {
        std::string testData4 = "Invalid@Base64!Data";
        std::string result4 = Task63::processRequest(testData4);
        std::cout << "Test 4 - Invalid Base64: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 Failed (Expected): " << e.what() << std::endl;
    }
    
    // Test Case 5: Invalid JSON
    try {
        std::string testData5 = Base64::encode("{invalid json}");
        std::string result5 = Task63::processRequest(testData5);
        std::cout << "Test 5 - Invalid JSON: " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 Failed (Expected): " << e.what() << std::endl;
    }
    
    return 0;
}
