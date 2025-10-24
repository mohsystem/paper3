#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

/**
 * Cleans up a user-entered phone number string.
 *
 * @param phoneNumber The raw phone number string.
 * @return A 10-digit NANP number string, or an empty string if the input is invalid.
 */
std::string cleanPhoneNumber(const std::string& phoneNumber) {
    std::string cleaned;
    
    // 1. Copy only digit characters to the cleaned string.
    std::copy_if(phoneNumber.begin(), phoneNumber.end(), std::back_inserter(cleaned),
                 [](char c) { return std::isdigit(c); });

    // 2. Handle country code if present.
    if (cleaned.length() == 11 && cleaned[0] == '1') {
        return cleaned.substr(1);
    }

    // 3. Check if the result is a 10-digit number.
    if (cleaned.length() == 10) {
        return cleaned;
    }
    
    // 4. If not, the number is invalid.
    return "";
}

int main() {
    std::string testCases[] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "12345" // Invalid case
    };

    for (const auto& testCase : testCases) {
        std::string result = cleanPhoneNumber(testCase);
        std::cout << "Input: \"" << testCase << "\", Cleaned: \"" << result << "\"" << std::endl;
    }

    return 0;
}