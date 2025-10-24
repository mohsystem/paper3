#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <numeric>

class Task3 {
public:
    /**
     * Checks if a string is a pangram.
     * A pangram is a sentence containing every letter of the alphabet at least once.
     * This function is case-insensitive and ignores numbers and punctuation.
     *
     * @param sentence The std::string to check.
     * @return true if the string is a pangram, false otherwise.
     */
    static bool isPangram(const std::string& sentence) {
        std::vector<bool> alphabetPresent(26, false);
        int uniqueLettersCount = 0;

        for (char ch : sentence) {
            // Check if the character is an alphabet character.
            if (std::isalpha(static_cast<unsigned char>(ch))) {
                // Convert to lowercase to handle both cases.
                char lower_ch = std::tolower(static_cast<unsigned char>(ch));
                int index = lower_ch - 'a';

                // If this letter hasn't been seen, mark it and increment the count.
                if (!alphabetPresent[index]) {
                    alphabetPresent[index] = true;
                    uniqueLettersCount++;
                }
            }
            
            // Optimization: if all 26 letters are found, no need to check further.
            if (uniqueLettersCount == 26) {
                return true;
            }
        }

        return uniqueLettersCount == 26;
    }
};

int main() {
    std::vector<std::string> testCases = {
        "The quick brown fox jumps over the lazy dog",
        "Pack my box with five dozen liquor jugs.",
        "This is not a pangram",
        "Abcdefghijklmnopqrstuvwxyz123!@#",
        ""
    };

    for (int i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ": \"" << testCases[i] << "\"" << std::endl;
        std::cout << "Is Pangram? " << (Task3::isPangram(testCases[i]) ? "true" : "false") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}