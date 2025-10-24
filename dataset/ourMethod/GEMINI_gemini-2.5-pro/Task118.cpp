#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

/**
 * @brief Validates if a given string is a valid IPv4 address.
 *
 * An IPv4 address is valid if it consists of four octets separated by dots,
 * where each octet is a decimal number from 0 to 255.
 * Leading zeros are not allowed (e.g., "01" is invalid), except for the number "0" itself.
 *
 * @param ip The string to validate.
 * @return true if the string is a valid IPv4 address, false otherwise.
 */
bool isValidIPv4(const std::string& ip) {
    // Rule #1: Ensure all input is validated.
    if (ip.empty()) {
        return false;
    }

    // Check for trailing dot, which getline would misinterpret
    if (ip.back() == '.') {
        return false;
    }

    std::stringstream ss(ip);
    std::string octet_str;
    int parts_count = 0;

    while (std::getline(ss, octet_str, '.')) {
        parts_count++;
        if (parts_count > 4) {
            return false; // Too many parts
        }

        if (octet_str.empty() || octet_str.length() > 3) {
            return false;
        }

        // Check for leading zeros, e.g., "01". "0" is allowed.
        if (octet_str.length() > 1 && octet_str[0] == '0') {
            return false;
        }

        for (char c : octet_str) {
            if (!std::isdigit(c)) {
                return false;
            }
        }
        
        // Rule #6: Handle potential exceptions from parsing.
        try {
            int octet_value = std::stoi(octet_str);
            if (octet_value < 0 || octet_value > 255) {
                return false;
            }
        } catch (const std::exception&) {
            // Should not be reached due to previous checks, but included for safety.
            return false;
        }
    }
    
    // Check if the stream had an error or if there are leftover characters
    if (ss.fail() && !ss.eof()) {
        return false;
    }

    return parts_count == 4;
}

int main() {
    const std::vector<std::string> testCases = {
        "192.168.1.1",       // Valid
        "256.0.0.1",         // Invalid range
        "192.168.01.1",      // Invalid leading zero
        "192.168.1.a",       // Invalid character
        "1.2.3"              // Invalid structure (too few parts)
    };

    for (const auto& ip : testCases) {
        std::cout << "Is \"" << ip << "\" a valid IPv4 address? " << (isValidIPv4(ip) ? "true" : "false") << std::endl;
    }

    std::cout << "\n--- Additional Test Cases ---" << std::endl;
    const std::vector<std::string> moreTestCases = {
        "0.0.0.0",           // Valid
        "255.255.255.255",   // Valid
        "1.2.3.4.5",         // Invalid structure (too many parts)
        "1..2.3.4",          // Invalid structure (empty octet)
        "1.2.3.",            // Invalid structure (trailing dot)
        "",                  // Invalid empty string
        "123.45.67.89"       // Valid
    };
    
    for (const auto& ip : moreTestCases) {
        std::cout << "Is \"" << ip << "\" a valid IPv4 address? " << (isValidIPv4(ip) ? "true" : "false") << std::endl;
    }

    return 0;
}