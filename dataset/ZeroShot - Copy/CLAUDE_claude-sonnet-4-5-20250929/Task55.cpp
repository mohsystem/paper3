
#include <iostream>
#include <string>
#include <regex>

bool isValidEmail(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    
    // Email regex pattern
    std::regex emailRegex("^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$");
    
    return std::regex_match(email, emailRegex);
}

int main() {
    // Test cases
    std::string testEmails[] = {
        "user@example.com",
        "john.doe@company.co.uk",
        "invalid.email@",
        "@invalid.com",
        "valid_email123@test-domain.org"
    };
    
    std::cout << "Email Validation Results:" << std::endl;
    for (const auto& email : testEmails) {
        std::cout << email << " -> " << (isValidEmail(email) ? "Valid" : "Invalid") << std::endl;
    }
    
    return 0;
}
