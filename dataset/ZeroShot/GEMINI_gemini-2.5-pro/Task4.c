#include <stdio.h>
#include <string.h>    // For strlen
#include <ctype.h>     // For tolower
#include <stdbool.h>   // For bool, true, false

/**
 * @brief Determines if a string is an isogram.
 * An isogram is a word that has no repeating letters. Case is ignored.
 * Assumes the input string contains only letters.
 * 
 * @param str The null-terminated string to check.
 * @return true if the string is an isogram, false otherwise.
 */
bool is_isogram(const char* str) {
    if (str == NULL) {
        return true; // Or handle as an error, but an empty concept is an isogram
    }

    // A boolean array to mark the presence of characters 'a' through 'z'.
    bool seen[26] = {false};
    
    for (size_t i = 0; i < strlen(str); i++) {
        // tolower expects an int and works with unsigned char values correctly
        char lower_c = tolower((unsigned char)str[i]);
        int index = lower_c - 'a';

        // If we have seen this character before, it's not an isogram.
        if (seen[index]) {
            return false;
        }
        // Mark this character as seen.
        seen[index] = true;
    }

    return true;
}

int main() {
    const char* testCases[] = {
        "Dermatoglyphics",
        "aba",
        "moOse",
        "",
        "isogram"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < numTestCases; i++) {
        bool result = is_isogram(testCases[i]);
        printf("\"%s\" --> %s\n", testCases[i], result ? "true" : "false");
    }

    return 0;
}