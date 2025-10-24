#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * @brief Cleans up user-entered phone numbers.
 * Removes punctuation and the country code (1) if present.
 * @param phoneNumber The raw phone number string.
 * @return A 10-digit phone number string, or an empty string if the input is not a valid NANP number.
 */
std::string cleanPhoneNumber(const std::string& phoneNumber) {
    std::string digits;
    for (char c : phoneNumber) {
        if (std::isdigit(c)) {
            digits += c;
        }
    }

    if (digits.length() == 11) {
        if (digits[0] == '1') {
            return digits.substr(1);
        }
    } else if (digits.length() == 10) {
        return digits;
    }

    // Return empty for invalid formats
    return "";
}

int main() {
    std::vector<std::string> testCases = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "(223) 456-7890"
    };

    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\", Output: \"" << cleanPhoneNumber(testCase) << "\"" << std::endl;
    }

    return 0;
}