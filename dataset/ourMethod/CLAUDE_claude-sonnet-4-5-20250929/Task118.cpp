
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <regex>
#include <limits>

// Validate IPv4 address format and range
// Returns true if valid, false otherwise
bool validateIPv4(const std::string& ip) {
    // Input validation: check length to prevent excessive processing
    if (ip.empty() || ip.length() > 15) { // Max IPv4: "255.255.255.255" = 15 chars
        return false;
    }
    
    // Check for valid characters only (digits and dots)
    for (char c : ip) {
        if (!std::isdigit(static_cast<unsigned char>(c)) && c != '.') {
            return false;
        }
    }
    
    // Count dots - must be exactly 3 for IPv4
    size_t dotCount = 0;
    for (char c : ip) {
        if (c == '.') dotCount++;
    }
    if (dotCount != 3) {
        return false;
    }
    
    // Parse octets safely
    std::vector<std::string> octets;
    std::stringstream ss(ip);
    std::string octet;
    
    while (std::getline(ss, octet, '.')) {
        octets.push_back(octet);
    }
    
    // Must have exactly 4 octets
    if (octets.size() != 4) {
        return false;
    }
    
    for (const auto& oct : octets) {
        // Empty octet is invalid
        if (oct.empty()) {
            return false;
        }
        
        // Leading zeros not allowed (except "0" itself)
        if (oct.length() > 1 && oct[0] == '0') {
            return false;
        }
        
        // Check length to prevent overflow
        if (oct.length() > 3) {
            return false;
        }
        
        // Safe string to integer conversion with overflow protection
        try {
            size_t pos = 0;
            long value = std::stol(oct, &pos);
            
            // Ensure entire string was consumed
            if (pos != oct.length()) {
                return false;
            }
            
            // Range check: 0-255 for IPv4 octets
            if (value < 0 || value > 255) {
                return false;
            }
        } catch (...) {
            // Conversion failed
            return false;
        }
    }
    
    return true;
}

// Validate IPv6 address format
// Returns true if valid, false otherwise
bool validateIPv6(const std::string& ip) {
    // Input validation: check length
    // Max IPv6: 8 groups of 4 hex digits with 7 colons = 39 chars
    // With :: compression, still reasonable limit
    if (ip.empty() || ip.length() > 45) {
        return false;
    }
    
    // Check for valid characters only (hex digits, colons)
    for (char c : ip) {
        if (!std::isxdigit(static_cast<unsigned char>(c)) && c != ':') {
            return false;
        }
    }
    
    // Count colons - should be 2-7 for valid IPv6 (or up to 9 with ::)
    size_t colonCount = 0;
    for (char c : ip) {
        if (c == ':') colonCount++;
    }
    if (colonCount < 2 || colonCount > 7) {
        // Exception: "::" compression can have more in edge cases
        if (ip.find("::") == std::string::npos) {
            return false;
        }
    }
    
    // Check for invalid patterns
    if (ip.find(":::") != std::string::npos) { // Triple colon invalid
        return false;
    }
    
    // "::" can appear only once
    size_t firstDoubleColon = ip.find("::");
    if (firstDoubleColon != std::string::npos) {
        size_t secondDoubleColon = ip.find("::", firstDoubleColon + 2);
        if (secondDoubleColon != std::string::npos) {
            return false; // Multiple "::" not allowed
        }
    }
    
    // Split by colons and validate groups
    std::vector<std::string> groups;
    std::stringstream ss(ip);
    std::string group;
    
    // Handle :: compression
    size_t doubleColonPos = ip.find("::");
    if (doubleColonPos != std::string::npos) {
        // Split into before and after ::
        std::string before = ip.substr(0, doubleColonPos);
        std::string after = doubleColonPos + 2 < ip.length() ? 
                           ip.substr(doubleColonPos + 2) : "";
        
        std::stringstream beforeSS(before);
        while (std::getline(beforeSS, group, ':')) {
            if (!group.empty()) groups.push_back(group);
        }
        
        std::stringstream afterSS(after);
        while (std::getline(afterSS, group, ':')) {
            if (!group.empty()) groups.push_back(group);
        }
        
        // With ::, groups should be < 8
        if (groups.size() >= 8) {
            return false;
        }
    } else {
        // No compression, must have exactly 8 groups
        while (std::getline(ss, group, ':')) {
            groups.push_back(group);
        }
        
        if (groups.size() != 8) {
            return false;
        }
    }
    
    // Validate each group
    for (const auto& grp : groups) {
        // Groups should not be empty (except with ::)
        if (grp.empty()) {
            continue;
        }
        
        // Groups should be 1-4 hex digits
        if (grp.length() > 4) {
            return false;
        }
        
        // Validate hex digits
        for (char c : grp) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
    }
    
    return true;
}

// Process and validate IP address (IPv4 or IPv6)
std::string processIPAddress(const std::string& ip) {
    // Input validation: check for null or empty
    if (ip.empty()) {
        return "Invalid: Empty IP address";
    }
    
    // Trim whitespace (simple implementation)
    std::string trimmedIP = ip;
    size_t start = trimmedIP.find_first_not_of(" \\t\\n\\r");
    size_t end = trimmedIP.find_last_not_of(" \\t\\n\\r");
    
    if (start == std::string::npos) {
        return "Invalid: Empty IP address";
    }
    
    trimmedIP = trimmedIP.substr(start, end - start + 1);
    
    // Detect IP version and validate
    if (trimmedIP.find(':') != std::string::npos) {
        // Likely IPv6
        if (validateIPv6(trimmedIP)) {
            return "Valid IPv6: " + trimmedIP;
        } else {
            return "Invalid IPv6: " + trimmedIP;
        }
    } else {
        // Likely IPv4
        if (validateIPv4(trimmedIP)) {
            return "Valid IPv4: " + trimmedIP;
        } else {
            return "Invalid IPv4: " + trimmedIP;
        }
    }
}

int main() {
    // Test cases with various valid and invalid IP addresses
    std::vector<std::string> testCases = {
        "192.168.1.1",           // Valid IPv4
        "256.1.1.1",             // Invalid IPv4 (out of range)
        "2001:0db8:85a3:0000:0000:8a2e:0370:7334",  // Valid IPv6
        "2001:db8::8a2e:370:7334",  // Valid IPv6 with compression
        "192.168.1.256"          // Invalid IPv4 (out of range)
    };
    
    std::cout << "IP Address Validation Results:\\n";
    std::cout << "================================\\n";
    
    for (const auto& testIP : testCases) {
        std::string result = processIPAddress(testIP);
        std::cout << result << std::endl;
    }
    
    return 0;
}
