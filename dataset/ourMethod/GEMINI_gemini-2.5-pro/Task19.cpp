#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

/**
 * @brief Reverses words in a string that have five or more letters.
 *
 * This function takes a string, finds words separated by spaces,
 * and reverses any word that is five or more characters long. The
 * original spacing is preserved.
 *
 * @param str The input string. Must consist of only letters and spaces.
 * @return A new string with the specified words reversed.
 */
std::string spinWords(const std::string &str) {
    std::string result = str;
    size_t word_start_idx = 0;

    for (size_t i = 0; i <= result.length(); ++i) {
        // A word is identified by a space or the end of the string
        if (i == result.length() || result[i] == ' ') {
            // Check if the word's length is 5 or more
            if (i - word_start_idx >= 5) {
                // Reverse the word in-place using iterators
                std::reverse(result.begin() + word_start_idx, result.begin() + i);
            }
            // The next word starts after the current space
            word_start_idx = i + 1;
        }
    }
    return result;
}

int main() {
    std::vector<std::string> test_cases = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "Just kidding there is still one more"
    };

    for (const auto& test_str : test_cases) {
        std::cout << "Original: \"" << test_str << "\"" << std::endl;
        std::string spun_str = spinWords(test_str);
        std::cout << "Spun:     \"" << spun_str << "\"" << std::endl << std::endl;
    }

    return 0;
}