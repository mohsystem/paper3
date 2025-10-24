#include <iostream>
#include <string>
#include <vector>
#include <regex>

/**
 * Validates an email address using a regular expression.
 *
 * @param email The email string to validate.
 * @return true if the email is valid, false otherwise.
 */
bool isValidEmail(const std::string& email) {
    // A common regex for email validation.
    // Note the double backslashes needed to escape in C++ raw string literals.
    const std::regex pattern(R"(^[a-zA-Z0-9_+&*-]+(?:\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\.)+[a-zA-Z]{2,7}$)");
    return std::regex_match(email, pattern);
}

int main() {
    std::vector<std::string> testEmails = {
        "test.user@example.com",      // Valid
        "user@sub.domain.co.uk",  // Valid
        "invalid-email",              // Invalid (no @)
        "user@.com",                  // Invalid (domain starts with a dot)
        "user@domain..com"            // Invalid (consecutive dots in domain)
    };
    
    std::cout << "--- C++ Email Validation ---" << std::endl;
    for (const auto& email : testEmails) {
        std::cout << "Email: " << email << " -> Is Valid: " << (isValidEmail(email) ? "true" : "false") << std::endl;
    }

    return 0;
}