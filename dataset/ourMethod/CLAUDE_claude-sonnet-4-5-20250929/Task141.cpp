
#include <iostream>
#include <string>
#include <algorithm>
#include <cstddef>

// Function to reverse a string safely
// Returns the reversed string
// Security: Uses std::string to avoid buffer overflows
std::string reverseString(const std::string& input) {
    // Validate input length to prevent excessive memory usage
    // Maximum reasonable string length: 10MB
    const size_t MAX_STRING_LENGTH = 10 * 1024 * 1024;
    
    if (input.length() > MAX_STRING_LENGTH) {
        throw std::length_error("Input string exceeds maximum allowed length");
    }
    
    // Create a copy to avoid modifying the input
    std::string reversed = input;
    
    // Use standard library function for safe reversal
    // std::reverse handles all edge cases and boundary checks internally
    std::reverse(reversed.begin(), reversed.end());
    
    return reversed;
}

int main() {
    // Test case 1: Normal string
    try {
        std::string test1 = "hello";
        std::string result1 = reverseString(test1);
        std::cout << "Input: \\"" << test1 << "\\" -> Output: \\"" << result1 << "\\"" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Empty string
    try {
        std::string test2 = "";
        std::string result2 = reverseString(test2);
        std::cout << "Input: \\"\\" -> Output: \\"" << result2 << "\\"" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Single character
    try {
        std::string test3 = "a";
        std::string result3 = reverseString(test3);
        std::cout << "Input: \\"" << test3 << "\\" -> Output: \\"" << result3 << "\\"" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 4: String with spaces and special characters
    try {
        std::string test4 = "Hello, World! 123";
        std::string result4 = reverseString(test4);
        std::cout << "Input: \\"" << test4 << "\\" -> Output: \\"" << result4 << "\\"" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Palindrome
    try {
        std::string test5 = "racecar";
        std::string result5 = reverseString(test5);
        std::cout << "Input: \\"" << test5 << "\\" -> Output: \\"" << result5 << "\\"" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
