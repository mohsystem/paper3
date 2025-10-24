
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

string cleanPhoneNumber(const string& phoneNumber) {
    if (phoneNumber.empty()) {
        return "";
    }
    
    // Remove all non-digit characters
    string digitsOnly;
    for (char c : phoneNumber) {
        if (isdigit(c)) {
            digitsOnly += c;
        }
    }
    
    // Check if empty after cleaning
    if (digitsOnly.empty()) {
        return "";
    }
    
    // Handle country code 1
    if (digitsOnly.length() == 11 && digitsOnly[0] == '1') {
        digitsOnly = digitsOnly.substr(1);
    }
    
    // Validate length
    if (digitsOnly.length() != 10) {
        return "";
    }
    
    // Validate NANP format: NXX NXX-XXXX
    // First digit of area code (N) must be 2-9
    if (digitsOnly[0] < '2' || digitsOnly[0] > '9') {
        return "";
    }
    
    // Fourth digit (first digit of exchange code) must be 2-9
    if (digitsOnly[3] < '2' || digitsOnly[3] > '9') {
        return "";
    }
    
    return digitsOnly;
}

int main() {
    // Test case 1: Phone number with country code and special characters
    cout << "Test 1: " << cleanPhoneNumber("+1 (613)-995-0253") << endl;
    
    // Test case 2: Phone number with dashes
    cout << "Test 2: " << cleanPhoneNumber("613-995-0253") << endl;
    
    // Test case 3: Phone number with country code and spaces
    cout << "Test 3: " << cleanPhoneNumber("1 613 995 0253") << endl;
    
    // Test case 4: Phone number with dots
    cout << "Test 4: " << cleanPhoneNumber("613.995.0253") << endl;
    
    // Test case 5: Phone number with parentheses
    cout << "Test 5: " << cleanPhoneNumber("(234) 567-8901") << endl;
    
    return 0;
}
