#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

/**
 * Validates if the given string is a valid IPv4 address.
 *
 * @param ip The string to validate.
 * @return true if the string is a valid IPv4 address, false otherwise.
 */
bool isValidIP(const std::string& ip) {
    std::stringstream ss(ip);
    std::string part;
    int part_count = 0;

    while (std::getline(ss, part, '.')) {
        part_count++;

        // Each part must not be empty and must have at most 3 digits.
        if (part.empty() || part.length() > 3) {
            return false;
        }

        // Each part must contain only digits.
        for (char c : part) {
            if (!std::isdigit(c)) {
                return false;
            }
        }
        
        // A part cannot have leading zeros unless it is "0" itself.
        if (part.length() > 1 && part[0] == '0') {
            return false;
        }

        // The isdigit check above prevents std::invalid_argument exceptions.
        // The length check prevents std::out_of_range exceptions.
        int num = std::stoi(part);

        // The number must be in the range [0, 255].
        if (num < 0 || num > 255) {
            return false;
        }
    }
    
    // An IPv4 address must have exactly 4 parts. The stringstream method alone
    // might not catch all formatting errors (e.g., a trailing dot).
    // A robust way to confirm the structure is to count the dots in the original string.
    int dot_count = 0;
    for (char c : ip) {
        if (c == '.') {
            dot_count++;
        }
    }

    return part_count == 4 && dot_count == 3;
}

int main() {
    std::vector<std::string> testCases = {
        "127.0.0.1",       // Valid
        "256.0.0.0",       // Invalid - range
        "192.168.01.1",    // Invalid - leading zero
        "1.2.3",           // Invalid - format/parts count
        "a.b.c.d"          // Invalid - non-numeric
    };

    for (const auto& testCase : testCases) {
        std::cout << "IP: \"" << testCase << "\" is " 
                  << (isValidIP(testCase) ? "Valid" : "Invalid") << std::endl;
    }

    return 0;
}