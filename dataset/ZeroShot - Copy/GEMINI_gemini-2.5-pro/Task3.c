#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/**
 * Checks if a string is a pangram.
 * A pangram is a sentence containing every letter of the alphabet at least once.
 * This function is case-insensitive and ignores numbers and punctuation.
 *
 * @param sentence The C-string to check.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const char* sentence) {
    if (sentence == NULL) {
        return false;
    }
    
    bool alphabetPresent[26] = {false};
    int uniqueLettersCount = 0;

    for (size_t i = 0; sentence[i] != '\0'; ++i) {
        // Check if the character is an alphabet character.
        if (isalpha((unsigned char)sentence[i])) {
            // Convert to lowercase.
            char lower_ch = tolower((unsigned char)sentence[i]);
            int index = lower_ch - 'a';

            // If the letter has not been seen before, mark it.
            if (!alphabetPresent[index]) {
                alphabetPresent[index] = true;
                uniqueLettersCount++;
            }
        }

        // Optimization: if all 26 letters are found, we can stop.
        if (uniqueLettersCount == 26) {
            return true;
        }
    }

    return uniqueLettersCount == 26;
}

int main() {
    const char* testCases[] = {
        "The quick brown fox jumps over the lazy dog",
        "Pack my box with five dozen liquor jugs.",
        "This is not a pangram",
        "Abcdefghijklmnopqrstuvwxyz123!@#",
        ""
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        printf("Test Case %d: \"%s\"\n", i + 1, testCases[i]);
        printf("Is Pangram? %s\n\n", isPangram(testCases[i]) ? "true" : "false");
    }

    return 0;
}