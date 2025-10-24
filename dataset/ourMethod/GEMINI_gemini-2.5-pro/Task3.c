#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Checks if a given string is a pangram.
 *
 * A pangram is a sentence containing every letter of the English alphabet at least once.
 * This function is case-insensitive and ignores numbers, punctuation, and whitespace.
 *
 * @param s The input C-style string to check. Must be null-terminated.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const char* s) {
    // Rule: Treat all inputs as untrusted. Check for NULL pointer.
    if (s == NULL) {
        return false;
    }
    
    // A boolean array to mark the presence of each letter of the alphabet.
    // Initializing with {false} sets all 26 elements to false.
    bool seen[26] = {false};

    // Iterate over each character in the input string until the null terminator.
    for (size_t i = 0; s[i] != '\0'; ++i) {
        // Using unsigned char for ctype functions is safer to avoid undefined behavior.
        unsigned char c = s[i];
        
        // Check if the character is an alphabet letter.
        if (isalpha(c)) {
            // Convert the character to lowercase to handle case-insensitivity.
            char lower_c = tolower(c);
            // Mark the corresponding letter as seen.
            // 'a' corresponds to index 0, 'b' to 1, and so on.
            seen[lower_c - 'a'] = true;
        }
    }
    
    // Check if all 26 letters have been seen.
    for (int i = 0; i < 26; ++i) {
        if (!seen[i]) {
            return false; // If any letter is missing, it's not a pangram.
        }
    }
    
    return true; // All letters were found.
}

int main() {
    // Test Case 1: A classic pangram
    const char* s1 = "The quick brown fox jumps over the lazy dog";
    printf("Test 1: \"%s\" -> %s\n", s1, isPangram(s1) ? "true" : "false");

    // Test Case 2: A non-pangram
    const char* s2 = "This is not a pangram";
    printf("Test 2: \"%s\" -> %s\n", s2, isPangram(s2) ? "true" : "false");

    // Test Case 3: Pangram with all letters, upper and lower case
    const char* s3 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    printf("Test 3: \"%s\" -> %s\n", s3, isPangram(s3) ? "true" : "false");

    // Test Case 4: Another common pangram
    const char* s4 = "Pack my box with five dozen liquor jugs.";
    printf("Test 4: \"%s\" -> %s\n", s4, isPangram(s4) ? "true" : "false");

    // Test Case 5: A string missing a few letters
    const char* s5 = "A quick brown fox jumps over the lazy cat";
    printf("Test 5: \"%s\" -> %s\n", s5, isPangram(s5) ? "true" : "false");
    
    return 0;
}