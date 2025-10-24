#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * @brief Checks if a given string is a pangram.
 * A pangram is a sentence containing every letter of the English alphabet at least once.
 * The check is case-insensitive and ignores numbers and punctuation.
 * 
 * @param s The input null-terminated string.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const char *s) {
    if (s == NULL) {
        return false;
    }
    
    bool seen[26] = {false};
    int count = 0;
    
    for (int i = 0; s[i] != '\0'; i++) {
        if (isalpha(s[i])) {
            int index = tolower(s[i]) - 'a';
            if (!seen[index]) {
                seen[index] = true;
                count++;
            }
        }
    }
    
    return count == 26;
}

int main() {
    const char* testCases[] = {
        "The quick brown fox jumps over the lazy dog",
        "Pack my box with five dozen liquor jugs.",
        "This is not a pangram",
        "The quick brown fox jumps over the lazy do",
        ""
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Input: \"%s\"\n", testCases[i]);
        printf("Is Pangram: %s\n\n", isPangram(testCases[i]) ? "true" : "false");
    }

    return 0;
}