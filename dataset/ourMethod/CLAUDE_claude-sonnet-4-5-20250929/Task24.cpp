
#include <string>
#include <cctype>

// Validates ATM PIN: must be exactly 4 or 6 digits, nothing else
// Security: Input validation prevents injection attacks and ensures type safety
bool validatePin(const std::string& pin) {
    // Check for null or empty input - fail closed
    if (pin.empty()) {
        return false;
    }
    
    // Get length - must be exactly 4 or 6
    size_t len = pin.length();
    if (len != 4 && len != 6) {
        return false;
    }
    
    // Validate each character is a digit [0-9]
    // This prevents any non-digit characters including letters, symbols, whitespace
    for (size_t i = 0; i < len; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(pin[i]))) {
            return false;
        }
    }
    
    return true;
}

// Test cases
#include <iostream>
int main() {
    // Test case 1: Valid 4-digit PIN
    std::cout << "Test 1 - '1234': " << (validatePin("1234") ? "true" : "false") << std::endl;
    
    // Test case 2: Invalid 5-digit PIN
    std::cout << "Test 2 - '12345': " << (validatePin("12345") ? "true" : "false") << std::endl;
    
    // Test case 3: Invalid PIN with letter
    std::cout << "Test 3 - 'a234': " << (validatePin("a234") ? "true" : "false") << std::endl;
    
    // Test case 4: Valid 6-digit PIN
    std::cout << "Test 4 - '123456': " << (validatePin("123456") ? "true" : "false") << std::endl;
    
    // Test case 5: Empty string
    std::cout << "Test 5 - '': " << (validatePin("") ? "true" : "false") << std::endl;
    
    return 0;
}
