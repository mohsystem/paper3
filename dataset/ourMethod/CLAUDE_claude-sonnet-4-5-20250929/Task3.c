
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Function to check if a string is a pangram
// Returns true if the string contains all 26 letters of the alphabet (case insensitive)
// Security: Input validation and bounds checking throughout
bool isPangram(const char* text) {
    // Input validation: check for NULL pointer
    if (text == NULL) {
        return false;
    }
    
    // Validate string length to prevent excessive processing
    // Maximum reasonable length for a sentence (prevent DoS)
    const size_t MAX_LENGTH = 1000000;
    size_t len = strlen(text);
    if (len == 0 || len > MAX_LENGTH) {
        return false;
    }
    
    // Track which letters we've seen (a-z)\n    // Security: Initialize all elements to prevent undefined behavior\n    bool letters_found[26];\n    memset(letters_found, 0, sizeof(letters_found));\n    \n    // Iterate through each character in the string\n    // Security: bounds checked with validated length\n    for (size_t i = 0; i < len; ++i) {\n        // Security: cast to unsigned char to handle all byte values safely\n        unsigned char c = (unsigned char)text[i];\n        \n        // Check if it's a letter
        if (isalpha(c)) {
            // Convert to lowercase for comparison
            // Security: tolower handles all values correctly when passed unsigned char
            unsigned char lower = (unsigned char)tolower(c);
            
            // Calculate index (0-25) for the letter
            // Security: explicit bounds check to prevent out-of-bounds access
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

int main(void) {
    // Test case 1: Classic pangram
    const char* test1 = "The quick brown fox jumps over the lazy dog";
    printf("Test 1: \\"%s\\" -> %s\\n", test1, isPangram(test1) ? "true" : "false");
    
    // Test case 2: Pangram with numbers and punctuation
    const char* test2 = "Pack my box with five dozen liquor jugs!!! 123";
    printf("Test 2: \\"%s\\" -> %s\\n", test2, isPangram(test2) ? "true" : "false");
    
    // Test case 3: Not a pangram (missing letters)
    const char* test3 = "Hello world";
    printf("Test 3: \\"%s\\" -> %s\\n", test3, isPangram(test3) ? "true" : "false");
    
    // Test case 4: Empty string
    const char* test4 = "";
    printf("Test 4: \\"%s\\" -> %s\\n", test4, isPangram(test4) ? "true" : "false");
    
    // Test case 5: All letters in mixed case
    const char* test5 = "AbCdEfGhIjKlMnOpQrStUvWxYz";
    printf("Test 5: \\"%s\\" -> %s\\n", test5, isPangram(test5) ? "true" : "false");
    
    return 0;
}
