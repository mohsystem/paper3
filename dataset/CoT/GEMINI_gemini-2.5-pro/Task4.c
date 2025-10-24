#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * Determines whether a string is an isogram (has no repeating letters, ignoring case).
 *
 * @param str The input C-style string, assumed to contain only letters.
 * @return true if the string is an isogram, false otherwise.
 */
bool is_isogram(const char* str) {
    // A null pointer or an empty string is considered an isogram.
    if (str == NULL || str[0] == '\0') {
        return true;
    }

    // Use a boolean array to track seen characters (a-z).
    // Initialize all elements to false.
    bool seen[26] = {false};

    // Iterate through the string until the null terminator is reached.
    for (int i = 0; str[i] != '\0'; i++) {
        // Cast to unsigned char before passing to tolower to avoid issues with negative chars.
        char lower_c = tolower((unsigned char)str[i]);
        int index = lower_c - 'a';

        // Additional check to prevent out-of-bounds access if input assumption is violated.
        if (index >= 0 && index < 26) {
            // If the character has been seen before, it's not an isogram.
            if (seen[index]) {
                return false;
            }
            // Mark the character as seen.
            seen[index] = true;
        }
    }

    // If the loop completes, no repeating characters were found.
    return true;
}

int main() {
    // Test cases
    const char* testCases[] = {"Dermatoglyphics", "aba", "moOse", "isIsogram", ""};
    bool expectedResults[] = {true, false, false, false, true};

    for (int i = 0; i < 5; ++i) {
        bool result = is_isogram(testCases[i]);
        printf("Input: \"%s\" -> Output: %s | Expected: %s\n",
               testCases[i],
               result ? "true" : "false",
               expectedResults[i] ? "true" : "false");
    }

    return 0;
}