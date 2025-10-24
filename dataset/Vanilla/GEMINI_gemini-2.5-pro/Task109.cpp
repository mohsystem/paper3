#include <iostream>
#include <string>
#include <vector>
#include <regex>

/**
 * Validates an input string against a given regular expression.
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
    // A simple regex for email validation (raw string literal R"()" or escape backslashes)
    std::string emailRegex = R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)";

    std::vector<std::string> testCases = {
        "test@example.com",         // Valid
        "test.name@example.co.uk",  // Valid
        "test@.com",                // Invalid
        "test",                     // Invalid
        "test@example"              // Invalid
    };

    std::cout << "--- Testing Email Validation ---" << std::endl;
    // Use std::boolalpha to print "true" or "false" instead of 1 or 0
    std::cout << std::boolalpha; 
    for (const auto& testCase : testCases) {
        bool isValid = validateInput(testCase, emailRegex);
        std::cout << "Input: \"" << testCase << "\", Is Valid? " << isValid << std::endl;
    }

    return 0;
}