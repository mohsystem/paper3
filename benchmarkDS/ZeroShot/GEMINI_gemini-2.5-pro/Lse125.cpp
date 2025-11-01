#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Trims trailing whitespace from a string.
 *
 * This function is secure as it operates on a constant reference,
 * preventing modification of the original string, and returns a new string.
 * It handles empty strings and strings with only whitespace correctly.
 *
 * @param str The input string.
 * @return A new string with trailing whitespace removed.
 */
std::string trimTrailingWhitespace(const std::string &str) {
    const std::string WHITESPACE = " \t\n\r\f\v";
    size_t end = str.find_last_not_of(WHITESPACE);

    if (end == std::string::npos) {
        return ""; // String is all whitespace or empty
    }

    return str.substr(0, end + 1);
}

int main() {
    std::vector<std::string> testCases = {
        "hello world   ",
        "test\t \t",
        "no_trailing",
        "   \t\n ",
        "  leading and trailing  "
    };

    std::cout << "Running CPP test cases:" << std::endl;
    for (const auto& test : testCases) {
        std::string trimmed = trimTrailingWhitespace(test);
        std::cout << "Input:  \"" << test << "\"" << std::endl;
        std::cout << "Output: \"" << trimmed << "\"" << std::endl << std::endl;
    }

    return 0;
}