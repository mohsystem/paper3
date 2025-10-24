#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Validates an input string against a given regular expression.
 *
 * @param inputString The string to validate.
 * @param regexPattern The regular expression pattern.
 * @return true if the input string matches the entire pattern, false otherwise.
 * @note It is assumed the regexPattern is from a trusted source to prevent ReDoS attacks.
 */
bool validateInput(const std::string& inputString, const std::string& regexPattern) {
    try {
        const std::regex pattern(regexPattern);
        return std::regex_match(inputString, pattern);
    } catch (const std::regex_error& e) {
        // Handle cases where the provided regex is invalid
        std::cerr << "Invalid regex pattern: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // Test Case 1: Valid Email
    std::string emailValid = "test@example.com";
    // In C++, backslashes in string literals must be escaped
    std::string emailRegex = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";
    std::cout << "1. Testing '" << emailValid << "' against email regex: " << (validateInput(emailValid, emailRegex) ? "Valid" : "Invalid") << std::endl;

    // Test Case 2: Invalid Email
    std::string emailInvalid = "test@example";
    std::cout << "2. Testing '" << emailInvalid << "' against email regex: " << (validateInput(emailInvalid, emailRegex) ? "Valid" : "Invalid") << std::endl;

    // Test Case 3: Valid US Phone Number
    std::string phoneValid = "123-456-7890";
    std::string phoneRegex = "^\\d{3}-\\d{3}-\\d{4}$";
    std::cout << "3. Testing '" << phoneValid << "' against phone regex: " << (validateInput(phoneValid, phoneRegex) ? "Valid" : "Invalid") << std::endl;

    // Test Case 4: Valid Password (alphanumeric, 8+ characters)
    std::string passValid = "Password123";
    std::string passRegex = "^[A-Za-z0-9]{8,}$";
    std::cout << "4. Testing '" << passValid << "' against password regex: " << (validateInput(passValid, passRegex) ? "Valid" : "Invalid") << std::endl;

    // Test Case 5: Invalid Password (too short)
    std::string passInvalid = "pass";
    std::cout << "5. Testing '" << passInvalid << "' against password regex: " << (validateInput(passInvalid, passRegex) ? "Valid" : "Invalid") << std::endl;

    return 0;
}