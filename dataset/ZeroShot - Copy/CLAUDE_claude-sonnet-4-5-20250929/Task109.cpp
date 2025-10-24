
#include <iostream>
#include <regex>
#include <string>

// Validate email address
bool validateEmail(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    std::regex emailRegex("^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$");
    return std::regex_match(email, emailRegex);
}

// Validate phone number (US format)
bool validatePhone(const std::string& phone) {
    if (phone.empty()) {
        return false;
    }
    std::regex phoneRegex("^(\\\\+1)?[-.\\\\s]?\\\\(?([0-9]{3})\\\\)?[-.\\\\s]?([0-9]{3})[-.\\\\s]?([0-9]{4})$");
    return std::regex_match(phone, phoneRegex);
}

// Validate password (min 8 chars, at least 1 uppercase, 1 lowercase, 1 digit, 1 special char)
bool validatePassword(const std::string& password) {
    if (password.empty()) {
        return false;
    }
    std::regex passwordRegex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$");
    return std::regex_match(password, passwordRegex);
}

// Validate URL
bool validateURL(const std::string& url) {
    if (url.empty()) {
        return false;
    }
    std::regex urlRegex("^(https?|ftp)://[^\\\\s/$.?#].[^\\\\s]*$", std::regex_constants::icase);
    return std::regex_match(url, urlRegex);
}

// Validate username (alphanumeric and underscore, 3-16 chars)
bool validateUsername(const std::string& username) {
    if (username.empty()) {
        return false;
    }
    std::regex usernameRegex("^[a-zA-Z0-9_]{3,16}$");
    return std::regex_match(username, usernameRegex);
}

int main() {
    std::cout << "=== Test Case 1: Email Validation ===" << std::endl;
    std::string email1 = "user@example.com";
    std::string email2 = "invalid.email@";
    std::cout << email1 << " is valid: " << std::boolalpha << validateEmail(email1) << std::endl;
    std::cout << email2 << " is valid: " << std::boolalpha << validateEmail(email2) << std::endl;
    
    std::cout << "\\n=== Test Case 2: Phone Validation ===" << std::endl;
    std::string phone1 = "(123) 456-7890";
    std::string phone2 = "123-456-7890";
    std::cout << phone1 << " is valid: " << std::boolalpha << validatePhone(phone1) << std::endl;
    std::cout << phone2 << " is valid: " << std::boolalpha << validatePhone(phone2) << std::endl;
    
    std::cout << "\\n=== Test Case 3: Password Validation ===" << std::endl;
    std::string pwd1 = "SecurePass123!";
    std::string pwd2 = "weak";
    std::cout << pwd1 << " is valid: " << std::boolalpha << validatePassword(pwd1) << std::endl;
    std::cout << pwd2 << " is valid: " << std::boolalpha << validatePassword(pwd2) << std::endl;
    
    std::cout << "\\n=== Test Case 4: URL Validation ===" << std::endl;
    std::string url1 = "https://www.example.com";
    std::string url2 = "not a url";
    std::cout << url1 << " is valid: " << std::boolalpha << validateURL(url1) << std::endl;
    std::cout << url2 << " is valid: " << std::boolalpha << validateURL(url2) << std::endl;
    
    std::cout << "\\n=== Test Case 5: Username Validation ===" << std::endl;
    std::string user1 = "john_doe123";
    std::string user2 = "ab";
    std::cout << user1 << " is valid: " << std::boolalpha << validateUsername(user1) << std::endl;
    std::cout << user2 << " is valid: " << std::boolalpha << validateUsername(user2) << std::endl;
    
    return 0;
}
