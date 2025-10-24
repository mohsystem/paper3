#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * Checks if a given string is a pangram.
 * A pangram is a sentence that contains every single letter of the alphabet at least once.
 * The check is case-insensitive and ignores numbers and punctuation.
 *
 * @param s The input C-style string.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const char* s) {
    if (s == NULL) {
        return false;
    }
    
    // A boolean array to mark the presence of each letter 'a' through 'z'.
    // Initialize all elements to false.
    bool alphabetSeen[26] = {false};
    int uniqueLettersSeen = 0;

    for (int i = 0; s[i] != '\0'; i++) {
        char c = s[i];

        // Check if the character is an alphabet letter.
        if (isalpha(c)) {
            // Convert to lowercase to handle case-insensitivity.
            char lower_c = tolower(c);
            int index = lower_c - 'a';

            // If this letter has not been seen before, mark it and increment the count.
            if (!alphabetSeen[index]) {
                alphabetSeen[index] = true;
                uniqueLettersSeen++;
            }
        }
    }

    // The string is a pangram if we have seen all 26 letters.
    return uniqueLettersSeen == 26;
}

int main() {
    const char* testCases[] = {
        "The quick brown fox jumps over the lazy dog",
        "This is not a pangram",
        "Pack my box with five dozen liquor jugs.",
        "Cwm fjord bank glyphs vext quiz",
        "A an B b C c"
    };

    for (int i = 0; i < 5; ++i) {
        const char* testCase = testCases[i];
        bool result = isPangram(testCase);
        printf("Test Case %d: \"%s\"\n", i + 1, testCase);
        printf("Is Pangram? %s\n\n", result ? "true" : "false");
    }

    return 0;
}