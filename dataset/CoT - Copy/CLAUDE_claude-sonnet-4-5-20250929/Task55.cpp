
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

class Task55 {
private:
    static const int MAX_EMAIL_LENGTH = 320;
    static const int MAX_LOCAL_LENGTH = 64;
    static const int MAX_DOMAIN_LENGTH = 255;
    
    static int countChar(const std::string& str, char c) {
        return std::count(str.begin(), str.end(), c);
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
public:
    static bool isValidEmail(const std::string& input) {
        // Input validation - prevent empty strings
        if (input.empty()) {
            return false;
        }
        
        std::string email = trim(input);
        
        // Check maximum length to prevent DoS
        if (email.length() > MAX_EMAIL_LENGTH) {
            return false;
        }
        
        // Check for exactly one @ symbol
        int atCount = countChar(email, '@');
        if (atCount != 1) {
            return false;
        }
        
        size_t atPosition = email.find('@');
        if (atPosition == 0 || atPosition == email.length() - 1) {
            return false;
        }
        
        // Split into local and domain parts
        std::string localPart = email.substr(0, atPosition);
        std::string domainPart = email.substr(atPosition + 1);
        
        // Validate lengths
        if (localPart.length() > MAX_LOCAL_LENGTH || domainPart.length() > MAX_DOMAIN_LENGTH) {
            return false;
        }
        
        // Validate local part - use simple pattern to prevent ReDoS
        std::regex localPattern("^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+$");
        if (!std::regex_match(localPart, localPattern)) {
            return false;
        }
        
        // Check for consecutive dots or leading/trailing dots
        if (localPart[0] == '.' || localPart[localPart.length() - 1] == '.' || 
            localPart.find("..") != std::string::npos) {
            return false;
        }
        
        // Validate domain part - use simple pattern to prevent ReDoS
        std::regex domainPattern("^[a-zA-Z0-9]([a-zA-Z0-9-]*[a-zA-Z0-9])?(\\\\.[a-zA-Z0-9]([a-zA-Z0-9-]*[a-zA-Z0-9])?)*$");
        if (!std::regex_match(domainPart, domainPattern)) {
            return false;
        }
        
        // Check for valid TLD (at least one dot in domain)
        if (domainPart.find('.') == std::string::npos) {
            return false;
        }
        
        // Validate TLD length (2-63 characters)
        size_t lastDot = domainPart.rfind('.');
        std::string tld = domainPart.substr(lastDot + 1);
        if (tld.length() < 2 || tld.length() > 63) {
            return false;
        }
        
        return true;
    }
};

int main() {
    // Test cases
    std::string testEmails[] = {
        "user@example.com",
        "test.email@domain.co.uk",
        "invalid.email@",
        "@invalid.com",
        "no-at-sign.com",
        "user@domain",
        "user..name@domain.com",
        ".user@domain.com",
        "user@.domain.com",
        std::string(65, 'a') + "@test.com"
    };
    
    std::cout << "Email Validation Test Results:" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    
    for (const auto& email : testEmails) {
        std::string displayEmail = email.length() > 50 ? email.substr(0, 47) + "..." : email;
        std::cout << displayEmail;
        for (size_t i = displayEmail.length(); i < 50; i++) std::cout << " ";
        std::cout << " : " << (Task55::isValidEmail(email) ? "VALID" : "INVALID") << std::endl;
    }
    
    return 0;
}
