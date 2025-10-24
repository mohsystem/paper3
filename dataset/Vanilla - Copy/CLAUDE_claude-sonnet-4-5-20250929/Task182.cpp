
#include <iostream>
#include <string>
#include <cctype>

std::string cleanPhoneNumber(const std::string& phoneNumber) {
    // Remove all non-digit characters
    std::string digitsOnly;
    for (char c : phoneNumber) {
        if (std::isdigit(c)) {
            digitsOnly += c;
        }
    }
    
    // Check if it starts with country code 1 and has 11 digits
    if (digitsOnly.length() == 11 && digitsOnly[0] == '1') {
        digitsOnly = digitsOnly.substr(1);
    }
    
    // Validate the length is 10 digits
    if (digitsOnly.length() != 10) {
        return "";
    }
    
    // Validate area code (first digit must be 2-9)
    if (digitsOnly[0] < '2' || digitsOnly[0] > '9') {
        return "";
    }
    
    // Validate exchange code (fourth digit must be 2-9)
    if (digitsOnly[3] < '2' || digitsOnly[3] > '9') {
        return "";
    }
    
    return digitsOnly;
}

int main() {
    // Test case 1
    std::cout << "Test 1: " << cleanPhoneNumber("+1 (613)-995-0253") << std::endl;
    
    // Test case 2
    std::cout << "Test 2: " << cleanPhoneNumber("613-995-0253") << std::endl;
    
    // Test case 3
    std::cout << "Test 3: " << cleanPhoneNumber("1 613 995 0253") << std::endl;
    
    // Test case 4
    std::cout << "Test 4: " << cleanPhoneNumber("613.995.0253") << std::endl;
    
    // Test case 5
    std::cout << "Test 5: " << cleanPhoneNumber("(234) 567-8901") << std::endl;
    
    return 0;
}
