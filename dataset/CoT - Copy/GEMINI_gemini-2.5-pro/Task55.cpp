#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Validates an email address using a regular expression.
 *
 * @param email The string to validate.
 * @return true if the email is valid, false otherwise.
 */
bool isValidEmail(const std::string& email) {
    // An empty string is not a valid email
    if (email.empty()) {
        return false;
    }

    // Regex for standard email validation using a raw string literal.
    const std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");

    // std::regex_match checks if the entire string matches the pattern.
    return std::regex_match(email, pattern);
}

int main() {
    std::vector<std::string> testEmails = {
        "test@example.com",          // Valid
        "test.name+alias@example.co.uk", // Valid
        "plainaddress",              // Invalid
        "@missing-local-part.com",   // Invalid
        "test@domain@domain.com"     // Invalid
    };

    std::cout << "CPP Email Validation:" << std::endl;
    for (const auto& email : testEmails) {
        std::cout << "'" << email << "' is valid: " << (isValidEmail(email) ? "true" : "false") << std::endl;
    }
    return 0;
}