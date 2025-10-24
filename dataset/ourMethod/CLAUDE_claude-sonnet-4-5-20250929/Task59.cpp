
#include <string>
#include <cctype>
#include <iostream>
#include <stdexcept>

// Security: Validate input is only ASCII letters and reasonable length
// Prevent potential integer overflow and excessive memory allocation
std::string accum(const std::string& s) {
    // Security: Validate input length to prevent excessive memory allocation
    // Maximum safe length to prevent integer overflow in size calculations
    const size_t MAX_INPUT_LENGTH = 10000;
    
    if (s.empty()) {
        return "";
    }
    
    if (s.length() > MAX_INPUT_LENGTH) {
        throw std::invalid_argument("Input string too long");
    }
    
    // Security: Validate all characters are ASCII letters (a-z, A-Z)
    for (size_t i = 0; i < s.length(); ++i) {
        char c = s[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            throw std::invalid_argument("Input contains invalid characters");
        }
    }
    
    // Security: Pre-calculate required size to prevent buffer overflows
    // Size = sum of (i+1) for each character + (n-1) separators
    // Formula: n*(n+1)/2 + (n-1) where n is string length
    size_t n = s.length();
    size_t required_size = (n * (n + 1)) / 2 + (n > 0 ? n - 1 : 0);
    
    // Security: Check for potential overflow in size calculation
    if (required_size > 100000000) { // 100MB limit
        throw std::invalid_argument("Result would be too large");
    }
    
    std::string result;
    result.reserve(required_size); // Pre-allocate exact size needed
    
    for (size_t i = 0; i < s.length(); ++i) {
        char c = s[i];
        
        // Add separator after first character
        if (i > 0) {
            result += '-';
        }
        
        // First occurrence: uppercase
        result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        
        // Remaining occurrences: lowercase, repeated (i+1) times total
        for (size_t j = 0; j < i; ++j) {
            result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    
    return result;
}

int main() {
    // Test case 1
    std::cout << "Test 1: " << accum("abcd") << std::endl;
    
    // Test case 2
    std::cout << "Test 2: " << accum("RqaEzty") << std::endl;
    
    // Test case 3
    std::cout << "Test 3: " << accum("cwAt") << std::endl;
    
    // Test case 4
    std::cout << "Test 4: " << accum("ZpglnRxqenU") << std::endl;
    
    // Test case 5
    std::cout << "Test 5: " << accum("a") << std::endl;
    
    return 0;
}
