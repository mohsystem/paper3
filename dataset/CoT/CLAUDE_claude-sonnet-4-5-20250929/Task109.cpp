
#include <iostream>
#include <regex>
#include <string>

// Validates email format
bool validateEmail(const std::string& email) {
    if (email.empty() || email.length() > 254) {
        return false;
    }
    // RFC 5322 compliant email pattern with length limits
    std::regex emailPattern(R"(^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$)");
    return std::regex_match(email, emailPattern);
}

// Validates phone number (US format)
bool validatePhoneNumber(const std::string& phone) {
    if (phone.empty()) {
        return false;
    }
    // US phone format: (123) 456-7890 or 123-456-7890 or 1234567890
    std::regex phonePattern(R"(^(\\+?1)?[\\s-]?\\(?[0-9]{3}\\)?[\\s-]?[0-9]{3}[\\s-]?[0-9]{4}$)");
    return std::regex_match(phone, phonePattern);
}

// Validates URL format
bool validateURL(const std::string& url) {
    if (url.empty() || url.length() > 2048) {
        return false;
    }
    std::regex urlPattern(R"(^(https?://)(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{1,256}\\.[a-zA-Z0-9()]{1,6}\\b([-a-zA-Z0-9()@:%_\\+.~#?&//=]*)$)");
    return std::regex_match(url, urlPattern);
}

// Validates strong password
bool validatePassword(const std::string& password) {
    if (password.empty() || password.length() < 8 || password.length() > 128) {
        return false;
    }
    // Password must contain uppercase, lowercase, digit, and special character
    std::regex passwordPattern(R"(^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$)");
    return std::regex_match(password, passwordPattern);
}

// Validates username (alphanumeric and underscore, 3-20 characters)
bool validateUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }
    std::regex usernamePattern(R"(^[a-zA-Z0-9_]{3,20}$)");
    return std::regex_match(username, usernamePattern);
}

int main() {
    std::cout << "=== User Input Validation Tests ===" << std::endl << std::endl;
    
    // Test Case 1: Valid inputs
    std::cout << "Test Case 1: Valid Inputs" << std::endl;
    std::cout << "Email 'user@example.com': " << std::boolalpha << validateEmail("user@example.com") << std::endl;
    std::cout << "Phone '123-456-7890': " << validatePhoneNumber("123-456-7890") << std::endl;
    std::cout << "URL 'https://www.example.com': " << validateURL("https://www.example.com") << std::endl;
    std::cout << "Password 'SecurePass123!': " << validatePassword("SecurePass123!") << std::endl;
    std::cout << "Username 'john_doe123': " << validateUsername("john_doe123") << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Invalid email
    std::cout << "Test Case 2: Invalid Email" << std::endl;
    std::cout << "Email 'invalid.email': " << validateEmail("invalid.email") << std::endl;
    std::cout << "Email 'user@': " << validateEmail("user@") << std::endl;
    std::cout << "Email '@example.com': " << validateEmail("@example.com") << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Invalid phone number
    std::cout << "Test Case 3: Invalid Phone Number" << std::endl;
    std::cout << "Phone '12345': " << validatePhoneNumber("12345") << std::endl;
    std::cout << "Phone 'abc-def-ghij': " << validatePhoneNumber("abc-def-ghij") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Invalid password (weak)
    std::cout << "Test Case 4: Invalid Password" << std::endl;
    std::cout << "Password 'weak': " << validatePassword("weak") << std::endl;
    std::cout << "Password 'NoSpecialChar1': " << validatePassword("NoSpecialChar1") << std::endl;
    std::cout << "Password 'nouppercas1!': " << validatePassword("nouppercas1!") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Invalid username
    std::cout << "Test Case 5: Invalid Username" << std::endl;
    std::cout << "Username 'ab': " << validateUsername("ab") << std::endl;
    std::cout << "Username 'user@name': " << validateUsername("user@name") << std::endl;
    std::cout << "Username 'this_is_way_too_long_username': " << validateUsername("this_is_way_too_long_username") << std::endl;
    
    return 0;
}
