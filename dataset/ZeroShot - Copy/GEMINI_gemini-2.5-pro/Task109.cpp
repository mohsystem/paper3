#include <iostream>
#include <string>
#include <vector>
#include <regex>

/**
 * Validates a string against a given regular expression pattern.
 *
 * @param input The string to validate.
 * @param regexPattern The regular expression pattern.
 * @return true if the input matches the pattern, false otherwise.
 */
bool validateInput(const std::string& input, const std::string& regexPattern) {
    try {
        std::regex pattern(regexPattern);
        return std::regex_match(input, pattern);
    } catch (const std::regex_error& e) {
        std::cerr << "Regex error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // Test Cases
    std::vector<std::pair<std::string, std::string>> test_data = {
        {"test@example.com", "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"},
        {"123-456-7890", "^\\d{3}-\\d{3}-\\d{4}$"},
        {"Alpha123", "^[a-zA-Z0-9]+$"},
        {"Alpha123!", "^[a-zA-Z0-9]+$"},
        {"not-an-email", "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"}
    };

    std::vector<bool> expected_results = {true, true, true, false, false};

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (size_t i = 0; i < test_data.size(); ++i) {
        const auto& input = test_data[i].first;
        const auto& pattern = test_data[i].second;
        bool expected = expected_results[i];

        bool result = validateInput(input, pattern);
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input: \"" << input << "\"" << std::endl;
        std::cout << "Pattern: \"" << pattern << "\"" << std::endl;
        std::cout << "Result: " << std::boolalpha << result << " | Expected: " << expected << std::endl;
        std::cout << "Status: " << ((result == expected) ? "Passed" : "Failed") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}