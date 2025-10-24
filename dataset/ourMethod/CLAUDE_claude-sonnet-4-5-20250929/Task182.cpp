
#include <string>
#include <cctype>
#include <stdexcept>
#include <iostream>

// Clean phone number according to NANP rules
// Returns cleaned 10-digit number or throws exception for invalid input
std::string cleanPhoneNumber(const std::string& input) {
    // Security: Validate input length to prevent excessive processing
    // Maximum reasonable input: +1 (999) 999-9999 with extra spaces = ~20 chars
    if (input.length() > 100) {
        throw std::invalid_argument("Input too long");
    }
    
    // Extract only digits from input
    std::string digits;
    digits.reserve(11); // Pre-allocate for efficiency (max 11 digits with country code)
    
    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        // Security: Only accept ASCII digits, reject any non-printable or extended chars
        if (c >= '0' && c <= '9') {
            digits += c;
            // Security: Prevent excessive digit accumulation
            if (digits.length() > 11) {
                throw std::invalid_argument("Too many digits");
            }
        } else if (c == '+' || c == '-' || c == '(' || c == ')' || 
                   c == '.' || c == ' ') {
            // Allow common punctuation and whitespace, but ignore them
            continue;
        } else if (!std::isprint(static_cast<unsigned char>(c))) {
            // Security: Reject non-printable characters
            throw std::invalid_argument("Invalid character in input");
        }
        // Other printable characters are silently ignored for flexibility
    }
    
    // Remove country code if present
    if (digits.length() == 11) {
        // Security: Validate country code is exactly '1'
        if (digits[0] != '1') {
            throw std::invalid_argument("Invalid country code (must be 1)");
        }
        digits = digits.substr(1);
    } else if (digits.length() != 10) {
        throw std::invalid_argument("Invalid phone number length");
    }
    
    // Validate NANP format: NXX NXX-XXXX
    // Area code (first 3 digits): N (2-9) X X (0-9)
    if (digits[0] < '2' || digits[0] > '9') {
        throw std::invalid_argument("Area code must start with digit 2-9");
    }
    
    // Exchange code (digits 3-5): N (2-9) X X (0-9)
    if (digits[3] < '2' || digits[3] > '9') {
        throw std::invalid_argument("Exchange code must start with digit 2-9");
    }
    
    return digits;
}

int main() {
    // Test case 1: Country code with punctuation
    try {
        std::string result1 = cleanPhoneNumber("+1 (613)-995-0253");
        std::cout << "Test 1: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: No country code with dashes
    try {
        std::string result2 = cleanPhoneNumber("613-995-0253");
        std::cout << "Test 2: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Country code with spaces
    try {
        std::string result3 = cleanPhoneNumber("1 613 995 0253");
        std::cout << "Test 3: " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Dots as separators
    try {
        std::string result4 = cleanPhoneNumber("613.995.0253");
        std::cout << "Test 4: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid - area code starts with 1
    try {
        std::string result5 = cleanPhoneNumber("1 (123) 456-7890");
        std::cout << "Test 5: " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 (expected to fail): " << e.what() << std::endl;
    }
    
    return 0;
}
