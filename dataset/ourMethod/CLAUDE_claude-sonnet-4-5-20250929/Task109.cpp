
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <limits>

// Validates email format
// Security: Uses strict regex pattern to prevent malformed input
bool validateEmail(const std::string& email) {
    // Check length to prevent DoS via excessive backtracking
    if (email.empty() || email.length() > 254) {
        return false;
    }
    
    // Strict email pattern - no special characters that could cause issues
    // Pattern limits complexity to prevent ReDoS attacks
    std::regex emailPattern(
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$",
        std::regex::optimize
    );
    
    try {
        return std::regex_match(email, emailPattern);
    } catch (const std::regex_error&) {
        // Security: Handle regex errors gracefully
        return false;
    }
}

// Validates phone number (US format)
// Security: Strict format validation prevents injection
bool validatePhone(const std::string& phone) {
    // Length check prevents excessive processing
    if (phone.empty() || phone.length() > 20) {
        return false;
    }
    
    // Accepts formats: (123) 456-7890, 123-456-7890, 1234567890
    std::regex phonePattern(
        "^(\\\\([0-9]{3}\\\\) |[0-9]{3}-)?[0-9]{3}-?[0-9]{4}$",
        std::regex::optimize
    );
    
    try {
        return std::regex_match(phone, phonePattern);
    } catch (const std::regex_error&) {
        return false;
    }
}

// Validates username
// Security: Alphanumeric only with limited special chars
bool validateUsername(const std::string& username) {
    // Enforce reasonable length limits
    if (username.empty() || username.length() < 3 || username.length() > 20) {
        return false;
    }
    
    // Only alphanumeric, underscore, hyphen - prevents injection
    std::regex usernamePattern(
        "^[a-zA-Z0-9_-]{3,20}$",
        std::regex::optimize
    );
    
    try {
        return std::regex_match(username, usernamePattern);
    } catch (const std::regex_error&) {
        return false;
    }
}

// Validates password strength
// Security: Enforces strong password requirements
bool validatePassword(const std::string& password) {
    // Reasonable length bounds
    if (password.empty() || password.length() < 8 || password.length() > 128) {
        return false;
    }
    
    try {
        // Must contain uppercase, lowercase, digit, and special char
        std::regex upperCase("[A-Z]", std::regex::optimize);
        std::regex lowerCase("[a-z]", std::regex::optimize);
        std::regex digit("[0-9]", std::regex::optimize);
        std::regex special("[!@#$%^&*(),.?\\":{}|<>]", std::regex::optimize);
        
        return std::regex_search(password, upperCase) &&
               std::regex_search(password, lowerCase) &&
               std::regex_search(password, digit) &&
               std::regex_search(password, special);
    } catch (const std::regex_error&) {
        return false;
    }
}

// Validates IP address (IPv4)
// Security: Strict format prevents malformed input
bool validateIPv4(const std::string& ip) {
    // Length check
    if (ip.empty() || ip.length() > 15) {
        return false;
    }
    
    // Pattern with proper octet boundaries
    std::regex ipPattern(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$",
        std::regex::optimize
    );
    
    try {
        return std::regex_match(ip, ipPattern);
    } catch (const std::regex_error&) {
        return false;
    }
}

int main() {
    // Test case 1: Email validation
    std::cout << "Test 1 - Email Validation:" << std::endl;
    std::string email1 = "user@example.com";
    std::string email2 = "invalid.email";
    std::cout << "  " << email1 << ": " << (validateEmail(email1) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << email2 << ": " << (validateEmail(email2) ? "Valid" : "Invalid") << std::endl;
    
    // Test case 2: Phone validation
    std::cout << "\\nTest 2 - Phone Validation:" << std::endl;
    std::string phone1 = "(123) 456-7890";
    std::string phone2 = "123-456-7890";
    std::string phone3 = "invalid-phone";
    std::cout << "  " << phone1 << ": " << (validatePhone(phone1) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << phone2 << ": " << (validatePhone(phone2) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << phone3 << ": " << (validatePhone(phone3) ? "Valid" : "Invalid") << std::endl;
    
    // Test case 3: Username validation
    std::cout << "\\nTest 3 - Username Validation:" << std::endl;
    std::string user1 = "john_doe123";
    std::string user2 = "ab";
    std::string user3 = "user@name";
    std::cout << "  " << user1 << ": " << (validateUsername(user1) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << user2 << ": " << (validateUsername(user2) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << user3 << ": " << (validateUsername(user3) ? "Valid" : "Invalid") << std::endl;
    
    // Test case 4: Password validation
    std::cout << "\\nTest 4 - Password Validation:" << std::endl;
    std::string pass1 = "Strong@Pass123";
    std::string pass2 = "weakpass";
    std::cout << "  Strong@Pass123: " << (validatePassword(pass1) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  weakpass: " << (validatePassword(pass2) ? "Valid" : "Invalid") << std::endl;
    
    // Test case 5: IP address validation
    std::cout << "\\nTest 5 - IPv4 Validation:" << std::endl;
    std::string ip1 = "192.168.1.1";
    std::string ip2 = "256.1.1.1";
    std::string ip3 = "10.0.0.1";
    std::cout << "  " << ip1 << ": " << (validateIPv4(ip1) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << ip2 << ": " << (validateIPv4(ip2) ? "Valid" : "Invalid") << std::endl;
    std::cout << "  " << ip3 << ": " << (validateIPv4(ip3) ? "Valid" : "Invalid") << std::endl;
    
    return 0;
}
