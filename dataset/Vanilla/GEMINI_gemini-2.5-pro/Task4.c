#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Determines if a string is an isogram (no repeating letters, case-insensitive).
 *
 * @param str The string to check.
 * @return true if the string is an isogram, false otherwise.
 */
bool is_isogram(const char *str) {
    if (str == NULL) {
        return true;
    }

    // A boolean array to act as a set for the 26 letters of the alphabet.
    // Initialized to all false.
    bool seen[26] = {false}; 

    for (int i = 0; str[i] != '\0'; i++) {
        // Ensure the character is an alphabet character.
        if (isalpha(str[i])) {
            // Convert to lowercase to handle case-insensitivity.
            char lower_char = tolower(str[i]);
            // Calculate the index (0-25) for the character.
            int index = lower_char - 'a';
            
            // If we have seen this character before, it's a duplicate.
            if (seen[index]) {
                return false;
            }
            // Mark this character as seen.
            seen[index] = true;
        }
    }
    
    // If we get through the whole string, it's an isogram.
    return true;
}

int main() {
    const char *testCases[] = {
        "Dermatoglyphics",
        "aba",
        "moOse",
        "",
        "isogram"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("C Test Cases:\n");
    for (int i = 0; i < num_tests; i++) {
        // Use a ternary operator to print "true" or "false" based on the boolean result.
        printf("\"%s\" --> %s\n", testCases[i], is_isogram(testCases[i]) ? "true" : "false");
    }

    return 0;
}