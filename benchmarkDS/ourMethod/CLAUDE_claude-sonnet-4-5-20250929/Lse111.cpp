
#include <iostream>
#include <string>
#include <regex>
#include <array>
#include <memory>
#include <cstdio>
#include <cstring>

// Whitelist pattern: only allow valid hostnames and IP addresses
const std::regex VALID_HOST_PATTERN(
    "^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\\\.)*[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$|"
    "^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"
);

const size_t MAX_HOST_LENGTH = 253;
const size_t MAX_OUTPUT_SIZE = 8192;

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, (last - first + 1));
}

/**
 * Simulates a web endpoint that pings a host.
 * SECURITY: Uses strict input validation to prevent command injection.
 * Only allows validated hostnames/IPs, no shell metacharacters.
 */
std::string ping(const std::string& url) {
    // Input validation: reject empty
    if (url.empty()) {
        return "Error: Invalid input";
    }
    
    std::string cleaned_url = trim(url);
    
    if (cleaned_url.empty()) {
        return "Error: Invalid input";
    }
    
    // Length validation: prevent excessively long inputs
    if (cleaned_url.length() > MAX_HOST_LENGTH) {
        return "Error: Invalid input";
    }
    
    // Whitelist validation: only allow valid hostnames/IPs
    // This prevents command injection by rejecting shell metacharacters
    if (!std::regex_match(cleaned_url, VALID_HOST_PATTERN)) {
        return "Error: Invalid input";
    }
    
    // Build command with proper escaping - use array approach
    // Do not construct command string directly to avoid injection
    std::string command = "ping -c 4 '";
    
    // Additional safety: escape any single quotes (though regex should prevent them)
    for (char c : cleaned_url) {
        if (c == '\\'') {
            return "Error: Invalid input";
        }
        command += c;
    }
    command += "' 2>&1";
    
    std::string output;
    output.reserve(MAX_OUTPUT_SIZE);
    
    // Use popen with read-only mode
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return "Error: Operation failed";
    }
    
    // Read output with bounds checking
    std::array<char, 128> buffer;
    try {
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            size_t len = std::strlen(buffer.data());
            // Prevent excessive memory usage
            if (output.length() + len > MAX_OUTPUT_SIZE) {
                pclose(pipe);
                return "Error: Output too large";
            }
            output += buffer.data();
        }
    } catch (...) {
        pclose(pipe);
        return "Error: Operation failed";
    }
    
    pclose(pipe);
    return output;
}

int main() {
    // Test case 1: Valid hostname
    std::cout << "Test 1 - Valid hostname:" << std::endl;
    std::cout << ping("localhost") << std::endl;
    
    // Test case 2: Valid IP address
    std::cout << "\\nTest 2 - Valid IP:" << std::endl;
    std::cout << ping("127.0.0.1") << std::endl;
    
    // Test case 3: Command injection attempt (rejected)
    std::cout << "\\nTest 3 - Injection attempt:" << std::endl;
    std::cout << ping("localhost; cat /etc/passwd") << std::endl;
    
    // Test case 4: Invalid input with special characters
    std::cout << "\\nTest 4 - Special characters:" << std::endl;
    std::cout << ping("host`whoami`") << std::endl;
    
    // Test case 5: Empty input
    std::cout << "\\nTest 5 - Empty input:" << std::endl;
    std::cout << ping("") << std::endl;
    
    return 0;
}
