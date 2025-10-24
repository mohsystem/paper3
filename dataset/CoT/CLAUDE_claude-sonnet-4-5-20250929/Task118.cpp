
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <cstdlib>

class Task118 {
public:
    /**
     * Validates and processes IPv4 addresses
     * Prevents injection attacks and ensures strict validation
     */
    static bool validateIPv4(const std::string& ip) {
        if (ip.empty()) {
            return false;
        }
        
        // Prevent excessively long inputs (DoS prevention)
        if (ip.length() > 15) {
            return false;
        }
        
        // Strict regex pattern for IPv4 validation
        std::regex ipv4Pattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        
        if (!std::regex_match(ip, ipv4Pattern)) {
            return false;
        }
        
        // Additional validation: split and check each octet
        std::vector<std::string> octets;
        std::stringstream ss(ip);
        std::string octet;
        
        while (std::getline(ss, octet, '.')) {
            octets.push_back(octet);
        }
        
        if (octets.size() != 4) {
            return false;
        }
        
        for (const auto& oct : octets) {
            try {
                int value = std::stoi(oct);
                if (value < 0 || value > 255) {
                    return false;
                }
                // Reject leading zeros (except for "0" itself)
                if (oct.length() > 1 && oct[0] == '0') {
                    return false;
                }
            } catch (...) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     * Validates and processes IPv6 addresses
     * Prevents injection attacks and ensures strict validation
     */
    static bool validateIPv6(const std::string& ip) {
        if (ip.empty()) {
            return false;
        }
        
        // Prevent excessively long inputs (DoS prevention)
        if (ip.length() > 39) {
            return false;
        }
        
        // Strict regex pattern for IPv6 validation
        std::regex ipv6Pattern(
            "^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$|"
            "^::([0-9a-fA-F]{1,4}:){0,6}[0-9a-fA-F]{1,4}$|"
            "^([0-9a-fA-F]{1,4}:){1}:([0-9a-fA-F]{1,4}:){0,5}[0-9a-fA-F]{1,4}$|"
            "^([0-9a-fA-F]{1,4}:){2}:([0-9a-fA-F]{1,4}:){0,4}[0-9a-fA-F]{1,4}$|"
            "^([0-9a-fA-F]{1,4}:){3}:([0-9a-fA-F]{1,4}:){0,3}[0-9a-fA-F]{1,4}$|"
            "^([0-9a-fA-F]{1,4}:){4}:([0-9a-fA-F]{1,4}:){0,2}[0-9a-fA-F]{1,4}$|"
            "^([0-9a-fA-F]{1,4}:){5}:([0-9a-fA-F]{1,4}:){0,1}[0-9a-fA-F]{1,4}$|"
            "^([0-9a-fA-F]{1,4}:){6}:[0-9a-fA-F]{1,4}$|"
            "^::$"
        );
        
        return std::regex_match(ip, ipv6Pattern);
    }
    
    /**
     * Determines IP version and validates accordingly
     * Returns: "Valid IPv4", "Valid IPv6", or "Invalid IP"
     */
    static std::string processIPAddress(std::string ip) {
        if (ip.empty()) {
            return "Invalid IP";
        }
        
        // Sanitize input - remove whitespace
        ip.erase(0, ip.find_first_not_of(" \\t\\n\\r"));
        ip.erase(ip.find_last_not_of(" \\t\\n\\r") + 1);
        
        // Prevent buffer overflow and DoS attacks
        if (ip.length() > 100) {
            return "Invalid IP";
        }
        
        if (validateIPv4(ip)) {
            return "Valid IPv4";
        } else if (validateIPv6(ip)) {
            return "Valid IPv6";
        } else {
            return "Invalid IP";
        }
    }
    
    /**
     * Normalizes IPv4 address (removes leading zeros safely)
     */
    static std::string normalizeIPv4(const std::string& ip) {
        if (!validateIPv4(ip)) {
            return "";
        }
        
        std::vector<std::string> octets;
        std::stringstream ss(ip);
        std::string octet;
        
        while (std::getline(ss, octet, '.')) {
            octets.push_back(octet);
        }
        
        std::string normalized;
        for (size_t i = 0; i < octets.size(); i++) {
            int value = std::stoi(octets[i]);
            normalized += std::to_string(value);
            if (i < octets.size() - 1) {
                normalized += ".";
            }
        }
        
        return normalized;
    }
};

int main() {
    std::cout << "=== IP Address Validation Tests ===\\n\\n";
    
    // Test Case 1: Valid IPv4
    std::string test1 = "192.168.1.1";
    std::cout << "Test 1: " << test1 << "\\n";
    std::cout << "Result: " << Task118::processIPAddress(test1) << "\\n";
    std::cout << "IPv4 Valid: " << (Task118::validateIPv4(test1) ? "true" : "false") << "\\n";
    std::cout << "Normalized: " << Task118::normalizeIPv4(test1) << "\\n\\n";
    
    // Test Case 2: Invalid IPv4 (out of range)
    std::string test2 = "256.168.1.1";
    std::cout << "Test 2: " << test2 << "\\n";
    std::cout << "Result: " << Task118::processIPAddress(test2) << "\\n";
    std::cout << "IPv4 Valid: " << (Task118::validateIPv4(test2) ? "true" : "false") << "\\n\\n";
    
    // Test Case 3: Valid IPv6
    std::string test3 = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
    std::cout << "Test 3: " << test3 << "\\n";
    std::cout << "Result: " << Task118::processIPAddress(test3) << "\\n";
    std::cout << "IPv6 Valid: " << (Task118::validateIPv6(test3) ? "true" : "false") << "\\n\\n";
    
    // Test Case 4: Invalid IP (malicious input attempt)
    std::string test4 = "192.168.1.1; DROP TABLE users;";
    std::cout << "Test 4: " << test4 << "\\n";
    std::cout << "Result: " << Task118::processIPAddress(test4) << "\\n";
    std::cout << "IPv4 Valid: " << (Task118::validateIPv4(test4) ? "true" : "false") << "\\n\\n";
    
    // Test Case 5: Edge case (localhost)
    std::string test5 = "127.0.0.1";
    std::cout << "Test 5: " << test5 << "\\n";
    std::cout << "Result: " << Task118::processIPAddress(test5) << "\\n";
    std::cout << "IPv4 Valid: " << (Task118::validateIPv4(test5) ? "true" : "false") << "\\n";
    std::cout << "Normalized: " << Task118::normalizeIPv4(test5) << "\\n\\n";
    
    return 0;
}
