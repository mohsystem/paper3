
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <regex>

class Task126 {
private:
    static std::map<std::string, std::string> ALLOWED_PAGES;
    
    static std::string sanitizeInput(const std::string& input) {
        std::string sanitized = input;
        
        // Convert to lowercase
        std::transform(sanitized.begin(), sanitized.end(), sanitized.begin(), ::tolower);
        
        // Remove special characters, keep only alphanumeric
        sanitized.erase(
            std::remove_if(sanitized.begin(), sanitized.end(),
                [](char c) { return !std::isalnum(c); }),
            sanitized.end()
        );
        
        return sanitized;
    }
    
public:
    static std::string redirectUser(const std::string& userInput) {
        if (userInput.empty()) {
            return "Error: Invalid input. Redirecting to default page: /home";
        }
        
        // Sanitize input
        std::string sanitizedInput = sanitizeInput(userInput);
        
        // Check against whitelist
        auto it = ALLOWED_PAGES.find(sanitizedInput);
        if (it != ALLOWED_PAGES.end()) {
            return "Redirecting to: " + it->second;
        } else {
            return "Error: Page not found. Redirecting to default page: /home";
        }
    }
};

// Initialize static member
std::map<std::string, std::string> Task126::ALLOWED_PAGES = {
    {"home", "/home"},
    {"about", "/about"},
    {"contact", "/contact"},
    {"services", "/services"},
    {"profile", "/profile"}
};

int main() {
    std::cout << "=== User Redirection System (C++) ===\\n" << std::endl;
    
    // Test cases
    std::string testCases[] = {
        "home",
        "about",
        "contact",
        "invalid_page",
        "../../../etc/passwd"
    };
    
    for (const auto& testCase : testCases) {
        std::cout << "Input: \\"" << testCase << "\\"" << std::endl;
        std::cout << "Result: " << Task126::redirectUser(testCase) << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
