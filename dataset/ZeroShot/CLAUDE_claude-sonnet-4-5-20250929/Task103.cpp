
#include <iostream>
#include <string>
#include <cctype>

using namespace std;

string checkPasswordStrength(const string& password) {
    if (password.empty()) {
        return "Weak: Password is empty";
    }
    
    bool hasMinLength = password.length() >= 8;
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    
    for (char c : password) {
        if (isupper(c)) hasUppercase = true;
        if (islower(c)) hasLowercase = true;
        if (isdigit(c)) hasDigit = true;
    }
    
    if (hasMinLength && hasUppercase && hasLowercase && hasDigit) {
        return "Strong";
    } else {
        string weakness = "Weak: ";
        if (!hasMinLength) weakness += "Less than 8 characters. ";
        if (!hasUppercase) weakness += "No uppercase letter. ";
        if (!hasLowercase) weakness += "No lowercase letter. ";
        if (!hasDigit) weakness += "No digit. ";
        return weakness;
    }
}

int main() {
    // Test cases
    cout << "Test 1: " << checkPasswordStrength("Pass123word") << endl;
    cout << "Test 2: " << checkPasswordStrength("weak") << endl;
    cout << "Test 3: " << checkPasswordStrength("NoDigits") << endl;
    cout << "Test 4: " << checkPasswordStrength("alllowercase123") << endl;
    cout << "Test 5: " << checkPasswordStrength("ALLUPPERCASE123") << endl;
    
    return 0;
}
