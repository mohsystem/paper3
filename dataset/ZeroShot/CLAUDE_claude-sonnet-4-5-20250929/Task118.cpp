
#include <iostream>
#include <string>
#include <regex>
#include <vector>

bool isValidIPv4(const std::string& ip) {
    if (ip.empty()) {
        return false;
    }
    
    std::regex ipv4Pattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(ip, ipv4Pattern);
}

bool isValidIPv6(const std::string& ip) {
    if (ip.empty()) {
        return false;
    }
    
    std::regex ipv6Pattern("^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$");
    std::regex ipv6CompressedPattern("^(([0-9a-fA-F]{1,4}:){0,7}[0-9a-fA-F]{0,4})?::([0-9a-fA-F]{1,4}:){0,7}[0-9a-fA-F]{0,4}$");
    
    return std::regex_match(ip, ipv6Pattern) || std::regex_match(ip, ipv6CompressedPattern);
}

std::string validateIP(const std::string& ip) {
    if (isValidIPv4(ip)) {
        return "Valid IPv4";
    } else if (isValidIPv6(ip)) {
        return "Valid IPv6";
    } else {
        return "Invalid IP";
    }
}

int main() {
    // Test cases
    std::vector<std::string> testCases = {
        "192.168.1.1",
        "256.1.1.1",
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
        "::1",
        "invalid.ip.address"
    };
    
    std::cout << "IP Address Validation Results:" << std::endl;
    std::cout << "================================" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "IP: " << testCase << std::endl;
        std::cout << "Result: " << validateIP(testCase) << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }
    
    return 0;
}
