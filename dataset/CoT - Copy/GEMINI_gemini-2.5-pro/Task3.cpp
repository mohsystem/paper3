#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * Checks if a given string is a pangram.
 * A pangram is a sentence that contains every single letter of the alphabet at least once.
 * The check is case-insensitive and ignores numbers and punctuation.
 *
 * @param s The input string.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const std::string& s) {
    // A boolean vector to mark the presence of each letter 'a' through 'z'.
    std::vector<bool> alphabetSeen(26, false);
    int uniqueLettersSeen = 0;

    for (char c : s) {
        // Check if the character is an alphabet letter.
        if (std::isalpha(c)) {
            // Convert to lowercase to handle case-insensitivity.
            char lower_c = std::tolower(c);
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
    std::string testCases[] = {
        "The quick brown fox jumps over the lazy dog",
        "This is not a pangram",
        "Pack my box with five dozen liquor jugs.",
        "Cwm fjord bank glyphs vext quiz",
        "A an B b C c"
    };

    for (int i = 0; i < 5; ++i) {
        const std::string& testCase = testCases[i];
        bool result = isPangram(testCase);
        std::cout << "Test Case " << (i + 1) << ": \"" << testCase << "\"" << std::endl;
        std::cout << "Is Pangram? " << (result ? "true" : "false") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}