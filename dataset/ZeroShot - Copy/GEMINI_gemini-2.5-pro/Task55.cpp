#include <iostream>
#include <string>
#include <regex>
#include <vector>

bool isValidEmail(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    // Basic length check according to RFCs
    if (email.length() > 254) {
        return false;
    }

    // Regex for validating emails (OWASP recommendation)
    // Using a raw string literal (R"()") to avoid escaping backslashes
    const std::regex pattern(R"(^[a-zA-Z0-9_+&*-]+(?:\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\.)+[a-zA-Z]{2,7}$)");

    return std::regex_match(email, pattern);
}

int main() {
    std::vector<std::string> testEmails = {
        "test@example.com",           // Valid
        "test.name@example.co.uk",  // Valid
        "test..name@example.com",   // Invalid (consecutive dots)
        "test@example",             // Invalid (no TLD)
        "plainaddress"              // Invalid (no '@' or domain)
    };

    std::cout << "CPP Email Validation:" << std::endl;
    for (const auto& email : testEmails) {
        std::cout << "'" << email << "' is valid: " << (isValidEmail(email) ? "true" : "false") << std::endl;
    }

    return 0;
}