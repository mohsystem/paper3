
#include <iostream>
#include <string>
#include <cctype>
#include <cstring>

// Function to check if a string is an isogram
// Returns true if no letter repeats (case-insensitive), false otherwise
bool isIsogram(const std::string& str) {
    // Empty string is considered an isogram per specification
    if (str.empty()) {
        return true;
    }
    
    // Input validation: check string length to prevent excessive processing
    // Max 26 unique letters in English alphabet
    if (str.length() > 26) {
        return false;
    }
    
    // Use a fixed-size array to track seen letters (a-z)
    // Initialize to false - no letters seen yet
    bool seen[26];
    std::memset(seen, false, sizeof(seen));
    
    // Iterate through each character with bounds checking
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        
        // Validate that character is a letter (input sanitization)
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            // Non-letter characters invalidate the input
            continue; // Skip non-letter characters as per problem statement
        }
        
        // Convert to lowercase for case-insensitive comparison
        // Cast to unsigned char to avoid undefined behavior with negative values
        char lower = std::tolower(static_cast<unsigned char>(c));
        
        // Calculate index (0-25) with bounds validation
        int index = lower - 'a';
        
        // Bounds check to prevent out-of-range access
        if (index < 0 || index >= 26) {
            continue; // Skip invalid characters
        }
        
        // Check if letter was already seen
        if (seen[index]) {
            return false; // Duplicate found, not an isogram
        }
        
        // Mark letter as seen
        seen[index] = true;
    }
    
    return true; // No duplicates found
}

int main() {
    // Test cases to verify functionality
    std::cout << "Test 1: \\"Dermatoglyphics\\" -> " << (isIsogram("Dermatoglyphics") ? "true" : "false") << std::endl;
    std::cout << "Test 2: \\"aba\\" -> " << (isIsogram("aba") ? "true" : "false") << std::endl;
    std::cout << "Test 3: \\"moOse\\" -> " << (isIsogram("moOse") ? "true" : "false") << std::endl;
    std::cout << "Test 4: \\"\\" -> " << (isIsogram("") ? "true" : "false") << std::endl;
    std::cout << "Test 5: \\"abcdefg\\" -> " << (isIsogram("abcdefg") ? "true" : "false") << std::endl;
    
    return 0;
}
