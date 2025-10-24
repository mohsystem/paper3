
#include <iostream>
#include <string>
#include <cctype>
#include <cstdint>

// Function to check if a string has equal number of 'x's and 'o's (case insensitive)
// Returns true if counts are equal (including when both are zero)
bool XO(const std::string& str) {
    // Input validation: check for reasonable string length to prevent excessive processing
    // Limit to 1MB to prevent resource exhaustion attacks
    const size_t MAX_LENGTH = 1024 * 1024;
    if (str.length() > MAX_LENGTH) {
        std::cerr << "Error: Input string exceeds maximum allowed length" << std::endl;
        return false;
    }
    
    // Use size_t for counters to handle large strings without overflow
    // Initialize counters to zero
    size_t x_count = 0;
    size_t o_count = 0;
    
    // Iterate through each character in the string
    // Using size_t for index to prevent integer overflow
    for (size_t i = 0; i < str.length(); ++i) {
        // Convert to lowercase for case-insensitive comparison
        // Using std::tolower from <cctype> which is safe for all char values
        char c = std::tolower(static_cast<unsigned char>(str[i]));
        
        // Count 'x' characters
        if (c == 'x') {
            // Check for potential overflow before incrementing
            if (x_count == SIZE_MAX) {
                std::cerr << "Error: Counter overflow detected" << std::endl;
                return false;
            }
            ++x_count;
        }
        // Count 'o' characters
        else if (c == 'o') {
            // Check for potential overflow before incrementing
            if (o_count == SIZE_MAX) {
                std::cerr << "Error: Counter overflow detected" << std::endl;
                return false;
            }
            ++o_count;
        }
        // All other characters are ignored as per requirements
    }
    
    // Return true if counts are equal (including both zero)
    return x_count == o_count;
}

int main() {
    // Test case 1: Equal x's and o's
    std::cout << "Test 1 - XO(\\"ooxx\\"): " << (XO("ooxx") ? "true" : "false") << std::endl;
    
    // Test case 2: Unequal x's and o's (more x's)\n    std::cout << "Test 2 - XO(\\"xooxx\\"): " << (XO("xooxx") ? "true" : "false") << std::endl;\n    \n    // Test case 3: Equal x's and o's with mixed case and other chars\n    std::cout << "Test 3 - XO(\\"ooxXm\\"): " << (XO("ooxXm") ? "true" : "false") << std::endl;\n    \n    // Test case 4: No x's or o's present (both zero - should return true)\n    std::cout << "Test 4 - XO(\\"zpzpzpp\\"): " << (XO("zpzpzpp") ? "true" : "false") << std::endl;\n    \n    // Test case 5: Unequal x's and o's (more o's)
    std::cout << "Test 5 - XO(\\"zzoo\\"): " << (XO("zzoo") ? "true" : "false") << std::endl;
    
    return 0;
}
