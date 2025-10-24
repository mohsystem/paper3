
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Function to check if a string is an isogram
// Returns true if no letter repeats (case-insensitive), false otherwise
bool isIsogram(const char* str) {
    // Input validation: check for NULL pointer
    if (str == NULL) {
        return false;
    }
    
    // Calculate string length with bounds checking
    size_t len = strlen(str);
    
    // Empty string is considered an isogram per specification
    if (len == 0) {
        return true;
    }
    
    // Input validation: check string length to prevent excessive processing
    // Max 26 unique letters in English alphabet
    if (len > 26) {
        return false;
    }
    
    // Use a fixed-size array to track seen letters (a-z)
    // Initialize to false - no letters seen yet
    bool seen[26];
    memset(seen, 0, sizeof(seen));
    
    // Iterate through each character with bounds checking
    for (size_t i = 0; i < len; ++i) {
        // Bounds check: ensure we don't read past string end\n        if (str[i] == '\\0') {\n            break;\n        }\n        \n        unsigned char c = (unsigned char)str[i];\n        \n        // Validate that character is a letter (input sanitization)\n        if (!isalpha(c)) {\n            // Skip non-letter characters as per problem statement\n            continue;\n        }\n        \n        // Convert to lowercase for case-insensitive comparison\n        // Cast to unsigned char to avoid undefined behavior with negative values\n        unsigned char lower = (unsigned char)tolower(c);\n        \n        // Calculate index (0-25) with bounds validation\n        int index = lower - 'a';
        
        // Bounds check to prevent out-of-range array access
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
    printf("Test 1: \\"Dermatoglyphics\\" -> %s\\n", isIsogram("Dermatoglyphics") ? "true" : "false");
    printf("Test 2: \\"aba\\" -> %s\\n", isIsogram("aba") ? "true" : "false");
    printf("Test 3: \\"moOse\\" -> %s\\n", isIsogram("moOse") ? "true" : "false");
    printf("Test 4: \\"\\" -> %s\\n", isIsogram("") ? "true" : "false");
    printf("Test 5: \\"abcdefg\\" -> %s\\n", isIsogram("abcdefg") ? "true" : "false");
    
    return 0;
}
