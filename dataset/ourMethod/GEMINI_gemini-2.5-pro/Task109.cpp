#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Validates a string against a given regular expression pattern.
 *
 * @param input The string to validate.
 * @param regexPattern The regular expression pattern.
 * @return true if the input matches the pattern, false otherwise.
 */
bool validateInput(const std::string& input, const std::string& regexPattern) {
    try {
        std::regex pattern(regexPattern);
        return std::regex_match(input, pattern);
    } catch (const std::regex_error& e) {
        // Log the exception in a real application
        std::cerr << "Regex error: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // Test cases
    const std::string emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$";
    const std::string phoneRegex = "^\\(?([0-9]{3})\\)?[-.\\s]?([0-9]{3})[-.\\s]?([0-9]{4})$";
    const std::string usernameRegex = "^[a-zA-Z0-9]{5,15}$";
    
    std::cout << std::boolalpha;

    // Test Case 1: Valid Email
    std::string validEmail = "test.user@example.com";
    bool isEmailValid = validateInput(validEmail, emailRegex);
    std::cout << "1. Is '" << validEmail << "' a valid email? " << isEmailValid << std::endl;

    // Test Case 2: Invalid Email
    std::string invalidEmail = "invalid-email@.com";
    bool isEmailInvalid = validateInput(invalidEmail, emailRegex);
    std::cout << "2. Is '" << invalidEmail << "' a valid email? " << isEmailInvalid << std::endl;

    // Test Case 3: Valid US Phone Number
    std::string validPhone = "(123) 456-7890";
    bool isPhoneValid = validateInput(validPhone, phoneRegex);
    std::cout << "3. Is '" << validPhone << "' a valid US phone number? " << isPhoneValid << std::endl;

    // Test Case 4: Valid Alphanumeric Username
    std::string validUsername = "user123";
    bool isUsernameValid = validateInput(validUsername, usernameRegex);
    std::cout << "4. Is '" << validUsername << "' a valid username? " << isUsernameValid << std::endl;

    // Test Case 5: Invalid Username (too short)
    std::string invalidUsername = "u1";
    bool isUsernameInvalid = validateInput(invalidUsername, usernameRegex);
    std::cout << "5. Is '" << invalidUsername << "' a valid username? " << isUsernameInvalid << std::endl;

    return 0;
}