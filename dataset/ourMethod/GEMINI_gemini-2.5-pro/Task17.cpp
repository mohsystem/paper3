#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * @brief Converts a string to Jaden Case.
 * 
 * In Jaden Case, the first letter of each word is capitalized.
 *
 * @param phrase The input string. It is passed by const reference for efficiency
 *               and to prevent modification.
 * @return A new string in Jaden Case. Returns an empty string if the input
 *         is empty.
 */
std::string toJadenCase(const std::string& phrase) {
    if (phrase.empty()) {
        return "";
    }

    std::string result = phrase;
    bool capitalizeNext = true;

    for (char& c : result) {
        // The cast to unsigned char is a safe practice for ctype functions
        // to handle all possible char values correctly.
        if (capitalizeNext && std::isalpha(static_cast<unsigned char>(c))) {
            c = std::toupper(static_cast<unsigned char>(c));
            capitalizeNext = false;
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            capitalizeNext = true;
        }
    }

    return result;
}

int main() {
    // 5 test cases
    std::vector<std::string> testCases = {
        "How can mirrors be real if our eyes aren't real",
        "most trees are blue",
        "a b c d e f g",
        "",
        "   leading and trailing spaces   "
    };

    for (const auto& test : testCases) {
        std::cout << "Original:      \"" << test << "\"" << std::endl;
        std::string jadenCased = toJadenCase(test);
        std::cout << "Jaden-Cased:   \"" << jadenCased << "\"" << std::endl;
        std::cout << "--------------------------------------------------------" << std::endl;
    }

    return 0;
}