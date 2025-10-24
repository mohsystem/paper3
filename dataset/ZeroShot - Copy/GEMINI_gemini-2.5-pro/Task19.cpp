#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

/**
 * Reverses words in a sentence that have five or more letters.
 *
 * @param str The input string containing words and spaces.
 * @return The modified string with long words reversed.
 */
std::string spinWords(const std::string &str) {
    std::stringstream ss(str);
    std::string word;
    std::string result;
    
    // The loop `ss >> word` automatically handles splitting by whitespace.
    while (ss >> word) {
        if (!result.empty()) {
            result += " ";
        }
        
        if (word.length() >= 5) {
            std::reverse(word.begin(), word.end());
        }
        result += word;
    }
    
    return result;
}

int main() {
    std::vector<std::string> testCases = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome to the jungle",
        "Supercalifragilisticexpialidocious"
    };

    for (const auto& test : testCases) {
        std::cout << spinWords(test) << std::endl;
    }
    
    return 0;
}