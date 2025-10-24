
#include <iostream>
#include <string>
#include <regex>

using namespace std;

struct ValidationResult {
    bool isValid;
    string message;
    
    ValidationResult(bool valid, const string& msg) : isValid(valid), message(msg) {}
};

ValidationResult validateEmail(const string& email) {
    regex emailPattern("^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$");
    
    if (regex_match(email, emailPattern)) {
        return ValidationResult(true, "Valid email address");
    } else {
        return ValidationResult(false, "Invalid email address");
    }
}

ValidationResult validatePhone(const string& phone) {
    regex phonePattern("^\\\\+?[1-9]\\\\d{1,14}$|^\\\\(?\\\\d{3}\\\\)?[-.\\\\s]?\\\\d{3}[-.\\\\s]?\\\\d{4}$");
    
    if (regex_match(phone, phonePattern)) {
        return ValidationResult(true, "Valid phone number");
    } else {
        return ValidationResult(false, "Invalid phone number");
    }
}

ValidationResult validatePassword(const string& password) {
    regex passwordPattern("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\\\d)(?=.*[@$!%*?&])[A-Za-z\\\\d@$!%*?&]{8,}$");
    
    if (regex_match(password, passwordPattern)) {
        return ValidationResult(true, "Valid password");
    } else {
        return ValidationResult(false, "Invalid password (must be 8+ chars with uppercase, lowercase, digit, and special character)");
    }
}

ValidationResult validateUsername(const string& username) {
    regex usernamePattern("^[a-zA-Z0-9_]{3,20}$");
    
    if (regex_match(username, usernamePattern)) {
        return ValidationResult(true, "Valid username");
    } else {
        return ValidationResult(false, "Invalid username (must be 3-20 alphanumeric characters or underscore)");
    }
}

ValidationResult validateURL(const string& url) {
    regex urlPattern("^(https?|ftp)://[^\\\\s/$.?#].[^\\\\s]*$", regex_constants::icase);
    
    if (regex_match(url, urlPattern)) {
        return ValidationResult(true, "Valid URL");
    } else {
        return ValidationResult(false, "Invalid URL");
    }
}

int main() {
    cout << "===== Test Case 1: Email Validation =====" << endl;
    ValidationResult result1 = validateEmail("user@example.com");
    cout << "Input: user@example.com" << endl;
    cout << "Result: " << (result1.isValid ? "true" : "false") << " - " << result1.message << endl;
    cout << endl;
    
    cout << "===== Test Case 2: Phone Validation =====" << endl;
    ValidationResult result2 = validatePhone("123-456-7890");
    cout << "Input: 123-456-7890" << endl;
    cout << "Result: " << (result2.isValid ? "true" : "false") << " - " << result2.message << endl;
    cout << endl;
    
    cout << "===== Test Case 3: Password Validation =====" << endl;
    ValidationResult result3 = validatePassword("SecurePass123!");
    cout << "Input: SecurePass123!" << endl;
    cout << "Result: " << (result3.isValid ? "true" : "false") << " - " << result3.message << endl;
    cout << endl;
    
    cout << "===== Test Case 4: Username Validation =====" << endl;
    ValidationResult result4 = validateUsername("john_doe123");
    cout << "Input: john_doe123" << endl;
    cout << "Result: " << (result4.isValid ? "true" : "false") << " - " << result4.message << endl;
    cout << endl;
    
    cout << "===== Test Case 5: URL Validation =====" << endl;
    ValidationResult result5 = validateURL("https://www.example.com");
    cout << "Input: https://www.example.com" << endl;
    cout << "Result: " << (result5.isValid ? "true" : "false") << " - " << result5.message << endl;
    
    return 0;
}
