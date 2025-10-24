#include <iostream>
#include <string>
#include <cctype>    // For isdigit
#include <algorithm> // For std::all_of

/**
 * @brief Validates an ATM PIN.
 * A valid PIN must be exactly 4 or 6 digits long and contain only digits.
 * 
 * @param pin The string to validate.
 * @return true if the PIN is valid, false otherwise.
 */
bool validatePin(const std::string& pin) {
    // Check if the length is either 4 or 6
    bool is_valid_length = (pin.length() == 4 || pin.length() == 6);
    if (!is_valid_length) {
        return false;
    }
    
    // Check if all characters are digits using std::all_of and a lambda
    // This prevents iteration over strings that already fail the length check.
    return std::all_of(pin.begin(), pin.end(), [](char c){
        return std::isdigit(static_cast<unsigned char>(c));
    });
}

int main() {
    std::cout << "Testing CPP version:" << std::endl;
    // Test Case 1: Valid 4-digit PIN
    std::cout << "\"1234\"   -->  " << (validatePin("1234") ? "true" : "false") << std::endl;
    // Test Case 2: Invalid length
    std::cout << "\"12345\"  -->  " << (validatePin("12345") ? "true" : "false") << std::endl;
    // Test Case 3: Invalid character
    std::cout << "\"a234\"   -->  " << (validatePin("a234") ? "true" : "false") << std::endl;
    // Test Case 4: Valid 6-digit PIN
    std::cout << "\"451352\" -->  " << (validatePin("451352") ? "true" : "false") << std::endl;
    // Test Case 5: Invalid characters and length
    std::cout << "\"-1234\"  -->  " << (validatePin("-1234") ? "true" : "false") << std::endl;

    return 0;
}