
#include <iostream>
#include <string>
#include <cctype>
#include <cstring>

// Function to check if a string is a pangram
// Returns true if the string contains all 26 letters of the alphabet (case insensitive)
// Security: Input validation ensures we only process valid strings
bool isPangram(const std::string& text) {
    // Input validation: check if string is empty
    if (text.empty()) {
        return false;
    }
    
    // Validate string length to prevent excessive processing
    // Maximum reasonable length for a sentence (prevent DoS)
    const size_t MAX_LENGTH = 1000000;
    if (text.length() > MAX_LENGTH) {
        return false;
    }
    
    // Track which letters we've seen (a-z)\n    // Using a boolean array instead of bitwise operations for clarity and safety\n    bool letters_found[26];\n    // Security: Initialize all elements to prevent undefined behavior\n    std::memset(letters_found, false, sizeof(letters_found));\n    \n    // Iterate through each character in the string\n    // Bounds are guaranteed by std::string iterator\n    for (size_t i = 0; i < text.length(); ++i) {\n        char c = text[i];\n        \n        // Convert to lowercase and check if it's a letter
        // Security: std::tolower handles all char values safely
        if (std::isalpha(static_cast<unsigned char>(c))) {
            // Convert to lowercase for comparison
            char lower = std::tolower(static_cast<unsigned char>(c));
            
            // Calculate index (0-25) for the letter
            // Security: bounds check ensures index is valid
            int index = lower - 'a';
            if (index >= 0 && index < 26) {
                letters_found[index] = true;
            }
        }
        // Ignore numbers, punctuation, and other characters as per requirements
    }
    
    // Check if all 26 letters were found
    // Security: fixed iteration count, no user-controlled loops
    for (int i = 0; i < 26; ++i) {
        if (!letters_found[i]) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test case 1: Classic pangram
    std::string test1 = "The quick brown fox jumps over the lazy dog";
    std::cout << "Test 1: \\"" << test1 << "\\" -> " 
              << (isPangram(test1) ? "true" : "false") << std::endl;
    
    // Test case 2: Pangram with numbers and punctuation
    std::string test2 = "Pack my box with five dozen liquor jugs!!! 123";
    std::cout << "Test 2: \\"" << test2 << "\\" -> " 
              << (isPangram(test2) ? "true" : "false") << std::endl;
    
    // Test case 3: Not a pangram (missing letters)
    std::string test3 = "Hello world";
    std::cout << "Test 3: \\"" << test3 << "\\" -> " 
              << (isPangram(test3) ? "true" : "false") << std::endl;
    
    // Test case 4: Empty string
    std::string test4 = "";
    std::cout << "Test 4: \\"" << test4 << "\\" -> " 
              << (isPangram(test4) ? "true" : "false") << std::endl;
    
    // Test case 5: All letters in mixed case
    std::string test5 = "AbCdEfGhIjKlMnOpQrStUvWxYz";
    std::cout << "Test 5: \\"" << test5 << "\\" -> " 
              << (isPangram(test5) ? "true" : "false") << std::endl;
    
    return 0;
}
