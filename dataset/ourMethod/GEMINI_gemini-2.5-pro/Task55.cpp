#include <iostream>
#include <string>
#include <vector>
#include <cctype>

// Function to check if a character is a valid special character in the local part
bool isLocalPartSpecialChar(char c) {
    return c == '.' || c == '_' || c == '+' || c == '-';
}

// Function that validates an email address
bool isValidEmail(const std::string& email) {
    // 1. Total length check
    if (email.length() > 256 || email.length() < 3) {
        return false;
    }

    // 2. Find the '@' symbol. Must be exactly one.
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || email.rfind('@') != atPos) {
        return false; // No '@' or more than one '@'
    }

    // 3. Split into local and domain parts
    std::string localPart = email.substr(0, atPos);
    std::string domainPart = email.substr(atPos + 1);

    // 4. Validate Local Part
    if (localPart.empty() || localPart.length() > 64) {
        return false;
    }
    if (isLocalPartSpecialChar(localPart.front()) || isLocalPartSpecialChar(localPart.back())) {
        return false; // Cannot start or end with a special character
    }
    for (size_t i = 0; i < localPart.length(); ++i) {
        char c = localPart[i];
        if (!isalnum(c) && !isLocalPartSpecialChar(c)) {
            return false; // Invalid character
        }
        if (isLocalPartSpecialChar(c) && i + 1 < localPart.length() && isLocalPartSpecialChar(localPart[i + 1])) {
            return false; // Consecutive special characters
        }
    }

    // 5. Validate Domain Part
    if (domainPart.empty() || domainPart.length() > 255) {
        return false;
    }
    // Domain part cannot start or end with a dot or hyphen
    if (domainPart.front() == '.' || domainPart.front() == '-' || domainPart.back() == '.' || domainPart.back() == '-') {
        return false;
    }

    size_t lastDotPos = std::string::npos;
    size_t labelStart = 0;
    for (size_t i = 0; i < domainPart.length(); ++i) {
        char c = domainPart[i];
        if (c == '.') {
            if (i == labelStart) return false; // Empty label (e.g., domain..com)
            
            // Check the label from labelStart to i-1
            if (domainPart[labelStart] == '-' || domainPart[i - 1] == '-') {
                return false; // Label starts or ends with a hyphen
            }
            if ((i - labelStart) > 63) return false; // Label too long

            lastDotPos = i;
            labelStart = i + 1;
        } else if (!isalnum(c) && c != '-') {
            return false; // Invalid character in domain
        }
    }

    if (lastDotPos == std::string::npos) return false; // Must have at least one dot

    // Validate the TLD (the last label)
    if (labelStart >= domainPart.length()) return false; // No TLD found (e.g., domain.)

    size_t tldLen = domainPart.length() - labelStart;
    if (tldLen < 2 || tldLen > 63) return false; // TLD length constraints

    for (size_t i = labelStart; i < domainPart.length(); ++i) {
        if (!isalpha(domainPart[i])) {
            return false; // TLD must be all letters
        }
    }

    return true;
}

int main() {
    const char* emailsToTest[] = {
        "valid.email@example.com",
        "invalid-email@",
        "@invalid.com",
        "another..invalid@email.com",
        "valid-user@sub.domain.co"
    };

    std::cout << "--- 5 Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::string email(emailsToTest[i]);
        std::cout << "Email: \"" << email << "\" -> " 
                  << (isValidEmail(email) ? "Valid" : "Invalid") << std::endl;
    }

    return 0;
}