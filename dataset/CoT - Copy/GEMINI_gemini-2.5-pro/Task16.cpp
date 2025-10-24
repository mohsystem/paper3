#include <iostream>
#include <string>
#include <vector>

/**
 * Checks if the first string ends with the second string.
 *
 * @param str The main string.
 * @param ending The string to check for at the end of str.
 * @return true if str ends with ending, false otherwise.
 */
bool solution(const std::string& str, const std::string& ending) {
    if (ending.length() > str.length()) {
        return false;
    }
    // Compare the end of str with the ending string
    return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
}

int main() {
    // Test cases
    std::vector<std::pair<std::pair<std::string, std::string>, bool>> test_cases = {
        {{"abc", "bc"}, true},
        {{"abc", "d"}, false},
        {{"samurai", "ai"}, true},
        {{"abc", ""}, true},
        {{"abc", "abcde"}, false}
    };

    for (int i = 0; i < test_cases.size(); ++i) {
        const auto& test_case = test_cases[i];
        std::string str_val = test_case.first.first;
        std::string end_val = test_case.first.second;
        bool expected = test_case.second;
        
        bool actual = solution(str_val, end_val);
        
        std::cout << "Test Case " << i + 1 << ": solution(\"" << str_val << "\", \"" << end_val << "\")" << std::endl;
        std::cout << "Expected: " << std::boolalpha << expected << std::endl;
        std::cout << "Actual: " << std::boolalpha << actual << std::endl;
        std::cout << std::endl;
    }

    return 0;
}