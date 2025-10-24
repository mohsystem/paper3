#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <algorithm>

/**
 * Cleans a phone number by removing punctuation and country code.
 * Validates the number against NANP rules.
 *
 * @param input The raw phone number string.
 * @return A 10-digit clean phone number string.
 * @throw std::invalid_argument if the number is invalid.
 */
std::string cleanPhoneNumber(const std::string& input) {
    std::string digits;
    std::copy_if(input.begin(), input.end(), std::back_inserter(digits),
                 [](char c) { return std::isdigit(c); });

    if (digits.length() == 11) {
        if (digits.front() == '1') {
            digits = digits.substr(1);
        } else {
            throw std::invalid_argument("11-digit number must start with 1.");
        }
    }

    if (digits.length() != 10) {
        throw std::invalid_argument("Number must be 10 digits.");
    }

    // Area code is NXX where N is 2-9
    if (digits[0] == '0' || digits[0] == '1') {
        throw std::invalid_argument("Area code cannot start with 0 or 1.");
    }
    // Exchange code is NXX where N is 2-9
    if (digits[3] == '0' || digits[3] == '1') {
        throw std::invalid_argument("Exchange code cannot start with 0 or 1.");
    }

    return digits;
}

int main() {
    std::vector<std::string> testCases = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "(223) 456-7890",
        // Invalid cases for testing robustness
        "1 (123) 456-7890", // Area code starts with 1
        "(223) 056-7890", // Exchange code starts with 0
        "2 (223) 456-7890", // Invalid 11-digit number
        "(223) 456-789",    // Too short
        "123456789012"      // Too long
    };

    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\" -> ";
        try {
            std::string cleaned = cleanPhoneNumber(testCase);
            std::cout << "Output: " << cleaned << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}