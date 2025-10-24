#include <iostream>
#include <string>
#include <vector>
#include <utility>

/**
 * @brief Checks if a string ends with a given suffix.
 *
 * @param str The main string to check.
 * @param ending The suffix to look for at the end of str.
 * @return true if str ends with ending, false otherwise.
 */
bool solution(const std::string& str, const std::string& ending) {
    if (ending.length() > str.length()) {
        return false;
    }
    // Compare the substring of `str` starting from the position
    // that would align with the start of `ending` if it were a suffix.
    return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
}

int main() {
    // Test cases
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"abc", "bc"},      // Expected: true
        {"abc", "d"},       // Expected: false
        {"samurai", "ai"},  // Expected: true
        {"testing", ""},    // Expected: true
        {"fails", "ails"}   // Expected: true
    };

    std::cout << "Running C++ test cases..." << std::endl;
    for (const auto& test : test_cases) {
        bool result = solution(test.first, test.second);
        std::cout << "solution(\"" << test.first << "\", \"" << test.second << "\") -> "
                  << (result ? "true" : "false") << std::endl;
    }

    return 0;
}