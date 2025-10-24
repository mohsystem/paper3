#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

/**
 * Reverses words in a string that have five or more letters.
 *
 * @param str The input string of one or more words.
 * @return The string with long words reversed.
 */
std::string spinWords(const std::string &str) {
    std::stringstream ss(str);
    std::string word;
    std::string result = "";
    bool firstWord = true;

    while (ss >> word) {
        if (!firstWord) {
            result += " ";
        }
        if (word.length() >= 5) {
            std::reverse(word.begin(), word.end());
        }
        result += word;
        firstWord = false;
    }

    return result;
}

int main() {
    std::vector<std::string> testCases = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "Just kidding there is still one more"
    };

    std::vector<std::string> expectedResults = {
        "Hey wollef sroirraw",
        "This is a test",
        "This is rehtona test",
        "emocleW",
        "Just gniddik ereht is llits one more"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::string result = spinWords(testCases[i]);
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Input:    \"" << testCases[i] << "\"" << std::endl;
        std::cout << "Output:   \"" << result << "\"" << std::endl;
        std::cout << "Expected: \"" << expectedResults[i] << "\"" << std::endl;
        std::cout << "Result: " << (result == expectedResults[i] ? "Passed" : "Failed") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}