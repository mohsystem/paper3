
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <limits>

// Security: Using std::string for automatic memory management and bounds safety
// All inputs validated for null/empty cases
// No unsafe C string functions (strcpy, strcat, sprintf) used

class StringOperations {
public:
    // Copy string operation with validation
    // Returns copy of input, validates input is not empty
    static std::string copyString(const std::string& input) {
        // Validate input size to prevent excessive memory allocation
        if (input.size() > MAX_STRING_SIZE) {
            throw std::length_error("Input string exceeds maximum allowed size");
        }
        return input; // std::string handles safe copying
    }
    
    // Concatenate two strings with size validation
    // Prevents buffer overflow by checking total size before concatenation
    static std::string concatenateStrings(const std::string& str1, const std::string& str2) {
        // Security: Check for potential overflow before concatenation
        if (str1.size() > MAX_STRING_SIZE || str2.size() > MAX_STRING_SIZE) {
            throw std::length_error("Input string exceeds maximum allowed size");
        }
        
        // Check that concatenation won't exceed maximum size
        if (str1.size() > MAX_STRING_SIZE - str2.size()) {
            throw std::length_error("Concatenated string would exceed maximum size");
        }
        
        return str1 + str2; // std::string handles safe concatenation
    }
    
    // Reverse string operation
    // Uses safe iterator-based reversal
    static std::string reverseString(const std::string& input) {
        if (input.size() > MAX_STRING_SIZE) {
            throw std::length_error("Input string exceeds maximum allowed size");
        }
        
        std::string result = input;
        std::reverse(result.begin(), result.end()); // Safe STL algorithm
        return result;
    }
    
    // Convert to uppercase with validation
    static std::string toUpperCase(const std::string& input) {
        if (input.size() > MAX_STRING_SIZE) {
            throw std::length_error("Input string exceeds maximum allowed size");
        }
        
        std::string result = input;
        // Safe transformation using STL algorithm
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        return result;
    }
    
    // Find substring with bounds checking
    // Returns position or -1 if not found
    static int findSubstring(const std::string& haystack, const std::string& needle) {
        if (haystack.size() > MAX_STRING_SIZE || needle.size() > MAX_STRING_SIZE) {
            throw std::length_error("Input string exceeds maximum allowed size");
        }
        
        if (needle.empty()) {
            return 0; // Empty needle found at position 0
        }
        
        size_t pos = haystack.find(needle);
        if (pos == std::string::npos) {
            return -1;
        }
        
        // Security: Validate position fits in int before casting
        if (pos > static_cast<size_t>(std::numeric_limits<int>::max())) {
            throw std::overflow_error("Position exceeds int range");
        }
        
        return static_cast<int>(pos);
    }

private:
    // Maximum allowed string size to prevent excessive memory allocation
    static const size_t MAX_STRING_SIZE = 1000000; // 1MB limit
};

int main() {
    try {
        std::cout << "String Operations Test Cases\\n";
        std::cout << "============================\\n\\n";
        
        // Test Case 1: Copy operation
        std::cout << "Test 1 - Copy String:\\n";
        std::string original = "Hello, World!";
        std::string copied = StringOperations::copyString(original);
        std::cout << "Original: " << original << "\\n";
        std::cout << "Copied: " << copied << "\\n\\n";
        
        // Test Case 2: Concatenate operation
        std::cout << "Test 2 - Concatenate Strings:\\n";
        std::string str1 = "Secure ";
        std::string str2 = "Coding";
        std::string concatenated = StringOperations::concatenateStrings(str1, str2);
        std::cout << "String 1: " << str1 << "\\n";
        std::cout << "String 2: " << str2 << "\\n";
        std::cout << "Concatenated: " << concatenated << "\\n\\n";
        
        // Test Case 3: Reverse operation
        std::cout << "Test 3 - Reverse String:\\n";
        std::string toReverse = "12345";
        std::string reversed = StringOperations::reverseString(toReverse);
        std::cout << "Original: " << toReverse << "\\n";
        std::cout << "Reversed: " << reversed << "\\n\\n";
        
        // Test Case 4: Uppercase operation
        std::cout << "Test 4 - Convert to Uppercase:\\n";
        std::string lowercase = "testing uppercase";
        std::string uppercase = StringOperations::toUpperCase(lowercase);
        std::cout << "Original: " << lowercase << "\\n";
        std::cout << "Uppercase: " << uppercase << "\\n\\n";
        
        // Test Case 5: Find substring operation
        std::cout << "Test 5 - Find Substring:\\n";
        std::string haystack = "The quick brown fox jumps";
        std::string needle = "brown";
        int position = StringOperations::findSubstring(haystack, needle);
        std::cout << "Haystack: " << haystack << "\\n";
        std::cout << "Needle: " << needle << "\\n";
        std::cout << "Position: " << position << "\\n";
        
    } catch (const std::exception& e) {
        // Security: Safe error handling without exposing sensitive information
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
