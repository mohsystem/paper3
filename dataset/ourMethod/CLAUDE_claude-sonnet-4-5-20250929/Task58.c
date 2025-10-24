
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Function to check if a string has equal number of 'x's and 'o's (case insensitive)
// Returns true if counts are equal (including when both are zero)
// Returns false on error (NULL input or excessive length)
bool XO(const char* str) {
    // Input validation: check for NULL pointer
    if (str == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to XO\\n");
        return false;
    }
    
    // Calculate string length safely
    // Limit to 1MB to prevent resource exhaustion attacks
    const size_t MAX_LENGTH = 1024 * 1024;
    size_t len = 0;
    
    // Manual length calculation with bound check to prevent excessive processing
    while (str[len] != '\\0') {
        if (len >= MAX_LENGTH) {
            fprintf(stderr, "Error: Input string exceeds maximum allowed length\\n");
            return false;
        }
        ++len;
    }
    
    // Use size_t for counters to handle large strings without overflow
    // Initialize counters to zero
    size_t x_count = 0;
    size_t o_count = 0;
    
    // Iterate through each character in the string
    // Using size_t for index to prevent integer overflow
    for (size_t i = 0; i < len; ++i) {
        // Convert to lowercase for case-insensitive comparison
        // Cast to unsigned char first to avoid undefined behavior with negative char values
        char c = (char)tolower((unsigned char)str[i]);
        
        // Count 'x' characters
        if (c == 'x') {
            // Check for potential overflow before incrementing
            if (x_count == SIZE_MAX) {
                fprintf(stderr, "Error: Counter overflow detected\\n");
                return false;
            }
            ++x_count;
        }
        // Count 'o' characters
        else if (c == 'o') {
            // Check for potential overflow before incrementing
            if (o_count == SIZE_MAX) {
                fprintf(stderr, "Error: Counter overflow detected\\n");
                return false;
            }
            ++o_count;
        }
        // All other characters are ignored as per requirements
    }
    
    // Return true if counts are equal (including both zero)
    return x_count == o_count;
}

int main(void) {
    // Test case 1: Equal x's and o's
    printf("Test 1 - XO(\\"ooxx\\"): %s\\n", XO("ooxx") ? "true" : "false");
    
    // Test case 2: Unequal x's and o's (more x's)\n    printf("Test 2 - XO(\\"xooxx\\"): %s\\n", XO("xooxx") ? "true" : "false");\n    \n    // Test case 3: Equal x's and o's with mixed case and other chars\n    printf("Test 3 - XO(\\"ooxXm\\"): %s\\n", XO("ooxXm") ? "true" : "false");\n    \n    // Test case 4: No x's or o's present (both zero - should return true)\n    printf("Test 4 - XO(\\"zpzpzpp\\"): %s\\n", XO("zpzpzpp") ? "true" : "false");\n    \n    // Test case 5: Unequal x's and o's (more o's)
    printf("Test 5 - XO(\\"zzoo\\"): %s\\n", XO("zzoo") ? "true" : "false");
    
    return 0;
}
