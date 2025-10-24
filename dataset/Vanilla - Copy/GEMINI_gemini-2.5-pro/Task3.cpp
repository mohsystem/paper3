#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * @brief Checks if a given string is a pangram.
 * A pangram is a sentence containing every letter of the English alphabet at least once.
 * The check is case-insensitive and ignores numbers and punctuation.
 * 
 * @param s The input string.
 * @return true if the string is a pangram, false otherwise.
 */
bool isPangram(const std::string& s) {
    std::vector<bool> seen(26, false);
    int count = 0;

    for (char c : s) {
        if (std::isalpha(c)) {
            int index = std::tolower(c) - 'a';
            if (!seen[index]) {
                seen[index] = true;
                count++;
            }
        }
    }

    return count == 26;
}

int main() {
    std::vector<std::string> testCases = {
        "The quick brown fox jumps over the lazy dog",
        "Pack my box with five dozen liquor jugs.",
        "This is not a pangram",
        "The quick brown fox jumps over the lazy do",
        ""
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& test : testCases) {
        std::cout << "Input: \"" << test << "\"" << std::endl;
        std::cout << "Is Pangram: " << (isPangram(test) ? "true" : "false") << std::endl << std::endl;
    }

    return 0;
}