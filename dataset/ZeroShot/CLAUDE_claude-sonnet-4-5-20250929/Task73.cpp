
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

bool isValidAdminPassword(const string& password) {
    if (password.empty() || password.length() < 12) {
        return false;
    }
    
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    string specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    for (char c : password) {
        if (isupper(c)) {
            hasUppercase = true;
        } else if (islower(c)) {
            hasLowercase = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (specialChars.find(c) != string::npos) {
            hasSpecial = true;
        }
    }
    
    // Check for common weak passwords
    string lowerPassword = password;
    transform(lowerPassword.begin(), lowerPassword.end(), lowerPassword.begin(), ::tolower);
    
    string weakPasswords[] = {"password", "admin", "123456", "qwerty", "letmein"};
    for (const string& weak : weakPasswords) {
        if (lowerPassword.find(weak) != string::npos) {
            return false;
        }
    }
    
    return hasUppercase && hasLowercase && hasDigit && hasSpecial;
}

int main() {
    // Test cases
    string testPasswords[] = {
        "Admin@123456",           // Valid
        "SecureP@ssw0rd!",       // Valid
        "weak",                   // Invalid - too short
        "NoSpecialChar123",       // Invalid - no special character
        "Admin@Password2024"      // Valid
    };
    
    cout << "Admin Password Validation Results:" << endl;
    cout << "=====================================" << endl;
    
    for (const string& password : testPasswords) {
        bool isValid = isValidAdminPassword(password);
        cout << "Password: " << password << " -> " << (isValid ? "VALID" : "INVALID") << endl;
    }
    
    return 0;
}
