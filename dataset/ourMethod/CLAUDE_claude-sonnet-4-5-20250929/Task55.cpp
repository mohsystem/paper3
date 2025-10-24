
#include <iostream>
#include <string>
#include <regex>
#include <cstdint>

// Email validation function - validates format according to RFC 5322 simplified rules
// Returns true if email is valid, false otherwise
// Security: No external processing, pure validation logic
bool validateEmail(const std::string& email) {
    // Check input length to prevent excessive regex processing (DoS prevention)
    const size_t MAX_EMAIL_LENGTH = 320; // RFC 5321: 64 (local) + 1 (@) + 255 (domain)
    const size_t MIN_EMAIL_LENGTH = 3;   // a@b minimum
    
    if (email.empty() || email.length() < MIN_EMAIL_LENGTH || email.length() > MAX_EMAIL_LENGTH) {
        return false;
    }
    
    // Check for null bytes (security: prevent injection attacks)
    if (email.find('\\0') != std::string::npos) {
        return false;
    }
    
    // Regex pattern for email validation (simplified RFC 5322)
    // Pattern explanation:
    // - Local part: alphanumeric, dots, hyphens, underscores (no consecutive dots)
    // - @ symbol (exactly one)
    // - Domain: alphanumeric, dots, hyphens (valid domain format)
    // - TLD: 2-63 characters
    const std::regex pattern(
        "^[a-zA-Z0-9]([a-zA-Z0-9._-]*[a-zA-Z0-9])?@"
        "[a-zA-Z0-9]([a-zA-Z0-9.-]*[a-zA-Z0-9])?\\\\."
        "[a-zA-Z]{2,63}$"
    );
    
    // Validate format using regex
    if (!std::regex_match(email, pattern)) {
        return false;
    }
    
    // Additional validation: check for exactly one @ symbol
    size_t at_count = 0;
    size_t at_pos = 0;
    for (size_t i = 0; i < email.length(); ++i) {
        if (email[i] == '@') {
            at_count++;
            at_pos = i;
        }
    }
    
    if (at_count != 1) {
        return false;
    }
    
    // Validate local part length (max 64 characters)
    if (at_pos > 64) {
        return false;
    }
    
    // Validate domain part length (max 255 characters)
    size_t domain_length = email.length() - at_pos - 1;
    if (domain_length > 255) {
        return false;
    }
    
    // Check for consecutive dots (not allowed)
    if (email.find("..") != std::string::npos) {
        return false;
    }
    
    // Check for leading/trailing dots in local and domain parts
    if (email[0] == '.' || email[at_pos - 1] == '.' || 
        email[at_pos + 1] == '.' || email[email.length() - 1] == '.') {
        return false;
    }
    
    return true;
}

int main() {
    // Test cases
    std::string test_cases[5] = {
        "user@example.com",           // Valid
        "test.email@domain.co.uk",    // Valid
        "invalid.email@",             // Invalid: no domain
        "@nodomain.com",              // Invalid: no local part
        "no.at.sign.com"              // Invalid: no @ symbol
    };
    
    std::cout << "Email Validation Results:\\n";
    std::cout << "========================\\n";
    
    for (int i = 0; i < 5; ++i) {
        // Security: Use const reference to avoid copying potentially large strings
        const std::string& email = test_cases[i];
        bool is_valid = validateEmail(email);
        
        // Safe output formatting with const format string
        std::cout << "Email: \\"" << email << "\\" - " 
                  << (is_valid ? "VALID" : "INVALID") << "\\n";
    }
    
    return 0;
}
