
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <cstdlib>

// Maximum length to prevent ReDoS and excessive resource consumption
const size_t MAX_TARGET_LENGTH = 100;

// URL decode helper function
std::string urlDecode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.length());
    
    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%') {
            if (i + 2 < encoded.length()) {
                // Convert hex to char
                char hex[3] = {encoded[i+1], encoded[i+2], '\\0'};
                char* end = nullptr;
                long value = std::strtol(hex, &end, 16);
                if (end == hex + 2) {
                    decoded.push_back(static_cast<char>(value));
                    i += 2;
                    continue;
                }
            }
        } else if (encoded[i] == '+') {
            decoded.push_back(' ');
        } else {
            decoded.push_back(encoded[i]);
        }
    }
    return decoded;
}

/**
 * Validates and processes a redirect target based on secure pattern matching.
 * 
 * @param targetQueryParam The target query parameter value (untrusted input)
 * @return The validated redirect URL or root path "/"
 */
std::string processRedirect(const std::string& targetQueryParam) {
    // Rule#3: Input validation - check for empty input
    if (targetQueryParam.empty()) {
        return "/";  // Fail closed - redirect to root
    }
    
    try {
        // Decode URL-encoded input to prevent bypass via encoding
        std::string target = urlDecode(targetQueryParam);
        
        // Rule#3: Validate length to prevent resource exhaustion
        if (target.length() > MAX_TARGET_LENGTH) {
            return "/";  // Fail closed
        }
        
        // Rule#3: Additional validation - reject directory traversal attempts
        if (target.find("..") != std::string::npos || 
            target.find("/") == 0 || 
            target.find("\\\\") == 0) {\n            return "/";  // Fail closed\n        }\n        \n        // Rule#3: Reject URL schemes to prevent open redirect to external sites\n        if (target.find("://") != std::string::npos || \n            target.find("//") == 0) {\n            return "/";  // Fail closed - no external redirects\n        }\n        \n        // Secure pattern: matches alphanumeric/hyphen characters followed by period\n        // Pattern is anchored to prevent partial matches\n        std::regex safePattern("^[a-zA-Z0-9-]+\\\\.$");\n        \n        if (std::regex_match(target, safePattern)) {\n            // Even if pattern matches, construct safe relative path\n            return "./" + target;\n        } else {\n            // Pattern does not match - redirect to root\n            return "/";\n        }\n        \n    } catch (const std::exception& e) {\n        // Rule#1: Fail closed on any error, log generically\n        std::cerr << "Redirect validation error occurred" << std::endl;\n        return "/";\n    }\n}\n\nint main() {\n    // Test case 1: Valid input matching pattern\n    std::string test1 = "test123.";\n    std::cout << "Test 1 input: '" << test1 << "' -> " << processRedirect(test1) << std::endl;\n    \n    // Test case 2: Valid input with hyphens\n    std::string test2 = "my-page.";\n    std::cout << "Test 2 input: '" << test2 << "' -> " << processRedirect(test2) << std::endl;\n    \n    // Test case 3: Invalid - missing period\n    std::string test3 = "test123";\n    std::cout << "Test 3 input: '" << test3 << "' -> " << processRedirect(test3) << std::endl;\n    \n    // Test case 4: Invalid - directory traversal attempt\n    std::string test4 = "../etc.";\n    std::cout << "Test 4 input: '" << test4 << "' -> " << processRedirect(test4) << std::endl;\n    \n    // Test case 5: Invalid - open redirect attempt\n    std::string test5 = "http://evil.com.";\n    std::cout << "Test 5 input: '" << test5 << "' -> " << processRedirect(test5) << std::endl;
    
    return 0;
}
