
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdint>

// Security: Function to tweak letters by shifting them according to an array
// - Input validation: checks null/empty inputs, array bounds, and valid characters
// - Memory safety: uses std::string and std::vector (RAII)
// - No undefined behavior: validates all array accesses and character operations
std::string tweakLetters(const std::string& str, const std::vector<int>& tweaks) {
    // Security: Validate inputs are not empty
    if (str.empty()) {
        throw std::invalid_argument("Input string cannot be empty");
    }
    
    if (tweaks.empty()) {
        throw std::invalid_argument("Tweaks array cannot be empty");
    }
    
    // Security: Validate array sizes match to prevent out-of-bounds access
    if (str.length() != tweaks.size()) {
        throw std::invalid_argument("String length must match tweaks array size");
    }
    
    // Security: Validate string length to prevent excessive memory usage
    const size_t MAX_LENGTH = 1000000; // 1MB limit
    if (str.length() > MAX_LENGTH) {
        throw std::invalid_argument("Input string exceeds maximum allowed length");
    }
    
    std::string result;
    result.reserve(str.length()); // Pre-allocate to avoid reallocation
    
    // Process each character with bounds checking
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        int tweak = tweaks[i];
        
        // Security: Validate character is a lowercase letter
        if (c < 'a' || c > 'z') {
            throw std::invalid_argument("Input must contain only lowercase letters");
        }
        
        // Security: Validate tweak value is within reasonable bounds
        if (tweak < -26 || tweak > 26) {
            throw std::invalid_argument("Tweak value must be between -26 and 26");
        }
        
        // Security: Safe modular arithmetic to handle wrapping
        // Convert to 0-25 range, apply tweak with wrap-around, convert back
        int pos = static_cast<int>(c - 'a');
        pos = (pos + tweak) % 26;
        
        // Security: Handle negative modulo results
        if (pos < 0) {
            pos += 26;
        }
        
        // Security: Bounds check before casting back to char
        if (pos < 0 || pos > 25) {
            throw std::runtime_error("Internal error: position out of bounds");
        }
        
        result += static_cast<char>('a' + pos);
    }
    
    return result;
}

// Test cases
int main() {
    try {
        // Test case 1
        std::vector<int> tweaks1 = {0, 1, -1, 0, -1};
        std::string result1 = tweakLetters("apple", tweaks1);
        std::cout << "Test 1: " << result1 << " (expected: aqold)" << std::endl;
        
        // Test case 2
        std::vector<int> tweaks2 = {0, 0, 0, -1};
        std::string result2 = tweakLetters("many", tweaks2);
        std::cout << "Test 2: " << result2 << " (expected: manx)" << std::endl;
        
        // Test case 3
        std::vector<int> tweaks3 = {1, 1, 1, 1, 1};
        std::string result3 = tweakLetters("rhino", tweaks3);
        std::cout << "Test 3: " << result3 << " (expected: sijop)" << std::endl;
        
        // Test case 4: wrap-around from 'z'
        std::vector<int> tweaks4 = {1, 1, 1};
        std::string result4 = tweakLetters("xyz", tweaks4);
        std::cout << "Test 4: " << result4 << " (expected: yza)" << std::endl;
        
        // Test case 5: wrap-around from 'a' backwards
        std::vector<int> tweaks5 = {-1, -1, -1};
        std::string result5 = tweakLetters("abc", tweaks5);
        std::cout << "Test 5: " << result5 << " (expected: zab)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
