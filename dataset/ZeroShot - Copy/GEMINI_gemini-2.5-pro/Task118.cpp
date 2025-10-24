#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

/**
 * @brief Validates if the given string is a valid IPv4 address.
 *
 * A valid IPv4 address consists of four octets separated by dots.
 * Each octet must be a number between 0 and 255.
 * Leading zeros are not allowed, except for the number "0" itself.
 *
 * @param ip The string to validate.
 * @return true if the string is a valid IPv4 address, false otherwise.
 */
bool isValidIP(const std::string& ip) {
    if (ip.empty()) {
        return false;
    }

    std::stringstream ss(ip);
    std::string part;
    std::vector<std::string> parts;
    
    // Split the string by '.'
    while (std::getline(ss, part, '.')) {
        parts.push_back(part);
    }
    
    // std::getline does not handle a trailing delimiter as an extra part,
    // so "1.2.3.4." would result in 4 parts. We check this explicitly.
    if (!ip.empty() && ip.back() == '.') {
        return false;
    }
    
    if (parts.size() != 4) {
        return false;
    }

    for (const auto& p : parts) {
        // Each part must not be empty (e.g., from "1..2.3.4").
        if (p.empty()) {
            return false;
        }

        // A part cannot have leading zeros unless it is just "0".
        if (p.length() > 1 && p[0] == '0') {
            return false;
        }
        
        // Each part must consist of digits only.
        for (char c : p) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }

        try {
            int num = std::stoi(p);
            if (num < 0 || num > 255) {
                return false;
            }
        } catch (...) {
            // Catches std::invalid_argument or std::out_of_range from stoi.
            // This is a safeguard; the digit check should prevent this.
            return false;
        }
    }
    
    return true;
}

int main() {
    std::vector<std::string> testCases = {
        "192.168.1.1",      // Valid
        "255.255.255.255",  // Valid
        "0.0.0.0",          // Valid
        "1.2.3.4",          // Valid
        "192.168.1.256",    // Invalid: octet > 255
        "192.168.01.1",     // Invalid: leading zero
        "192.168.1",        // Invalid: too few parts
        "a.b.c.d",          // Invalid: non-numeric parts
        "1.2.3.4.",         // Invalid: trailing dot
        ".1.2.3.4",         // Invalid: leading dot
        "1..2.3.4"          // Invalid: consecutive dots
    };

    for (const auto& ip : testCases) {
        std::cout << "IP: " << ip << " is " << (isValidIP(ip) ? "valid" : "invalid") << std::endl;
    }
    return 0;
}