
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

// Allowlist of valid page destinations to prevent open redirect vulnerabilities
const std::unordered_map<std::string, std::string> VALID_PAGES = {
    {"home", "/home.html"},
    {"about", "/about.html"},
    {"contact", "/contact.html"},
    {"services", "/services.html"},
    {"products", "/products.html"}
};

const std::string DEFAULT_PAGE = "/home.html";

// Validate and sanitize user input to prevent injection attacks
std::string sanitizeInput(const std::string& input) {
    // Input length validation - prevent excessive input
    if (input.empty() || input.length() > 50) {
        return "";
    }
    
    std::string sanitized;
    sanitized.reserve(input.length());
    
    // Only allow alphanumeric characters and underscores
    for (char c : input) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
            sanitized += std::tolower(static_cast<unsigned char>(c));
        }
    }
    
    return sanitized;
}

// Redirect user based on validated input using allowlist approach
std::string redirectUser(const std::string& userInput) {
    // Sanitize input to prevent injection attacks
    std::string cleanInput = sanitizeInput(userInput);
    
    // Check if input is valid after sanitization
    if (cleanInput.empty()) {
        return DEFAULT_PAGE;
    }
    
    // Use allowlist to validate destination - prevents open redirect (CWE-601)
    auto it = VALID_PAGES.find(cleanInput);
    if (it != VALID_PAGES.end()) {
        return it->second;
    }
    
    // Default to safe page if input not in allowlist
    return DEFAULT_PAGE;
}

int main() {
    // Test cases demonstrating secure redirect functionality
    std::string testCases[] = {
        "home",           // Valid input
        "about",          // Valid input
        "products",       // Valid input
        "../etc/passwd",  // Path traversal attempt - blocked
        "javascript:alert(1)" // XSS attempt - blocked
    };
    
    std::cout << "Secure Page Redirect System\\n";
    std::cout << "===========================\\n\\n";
    
    for (const auto& testCase : testCases) {
        std::string destination = redirectUser(testCase);
        std::cout << "Input: \\"" << testCase << "\\" -> Redirect to: " 
                  << destination << "\\n";
    }
    
    return 0;
}
