#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * @brief Cleans up a user-entered phone number string by removing punctuation
 * and the NANP country code (1) if present.
 *
 * @param phoneNumber The raw phone number string.
 * @return A 10-digit phone number string if the input is a valid NANP number,
 *         otherwise an empty string.
 */
std::string cleanPhoneNumber(const std::string& phoneNumber) {
    std::string digits;
    for (char c : phoneNumber) {
        if (isdigit(c)) {
            digits.push_back(c);
        }
    }

    if (digits.length() == 11) {
        if (digits[0] == '1') {
            return digits.substr(1);
        }
    } else if (digits.length() == 10) {
        return digits;
    }

    // Return empty string for invalid lengths or invalid country codes
    return "";
}

int main() {
    std::vector<std::string> testCases = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "2 (123) 456-7890" // Invalid country code
    };

    for (const auto& testCase : testCases) {
        std::string cleanedNumber = cleanPhoneNumber(testCase);
        std::cout << "Original: \"" << testCase << "\", Cleaned: \"" << cleanedNumber << "\"" << std::endl;
    }

    return 0;
}