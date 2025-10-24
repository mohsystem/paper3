#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Converts a positive integer into its expanded form string.
 *
 * This function takes a positive integer and returns a string that represents
 * the number as a sum of its parts, each multiplied by its place value.
 * For example, 70304 becomes "70000 + 300 + 4".
 * The function handles input validation by returning an empty string for non-positive numbers,
 * although the problem statement guarantees positive integers.
 *
 * @param n The positive integer to convert. Must be greater than 0.
 * @return A std::string representing the expanded form.
 */
std::string expandedForm(long long n) {
    if (n <= 0) {
        return "";
    }
    std::string s = std::to_string(n);
    std::string result = "";
    bool first_term = true;
    size_t len = s.length();

    for (size_t i = 0; i < len; ++i) {
        if (s[i] != '0') {
            if (!first_term) {
                result += " + ";
            }
            result += s[i];
            for (size_t j = 0; j < len - 1 - i; ++j) {
                result += '0';
            }
            first_term = false;
        }
    }
    return result;
}

int main() {
    // Test cases
    long long test_cases[] = {12, 42, 70304, 999, 8};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        long long test_num = test_cases[i];
        std::cout << "Input: " << test_num << std::endl;
        std::cout << "Output: \"" << expandedForm(test_num) << "\"" << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}