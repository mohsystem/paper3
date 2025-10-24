
#include <iostream>
#include <cstring>
#include <algorithm>
#include <string>

#define BUFFER_SIZE 100

/**
 * Safely handles user input into a fixed-size buffer
 * @param input The user input string
 * @param buffer The output buffer (must be BUFFER_SIZE or larger)
 * @return The length of the copied string
 */
int handleInputToBuffer(const char* input, char* buffer) {
    if (input == nullptr || buffer == nullptr) {
        if (buffer != nullptr) {
            buffer[0] = '\\0';
        }
        return 0;
    }
    
    // Initialize buffer with null bytes
    memset(buffer, 0, BUFFER_SIZE);
    
    // Calculate safe copy length
    size_t inputLen = strlen(input);
    size_t copyLength = std::min(inputLen, static_cast<size_t>(BUFFER_SIZE - 1));
    
    // Safely copy with bounds checking
    strncpy(buffer, input, copyLength);
    
    // Ensure null termination
    buffer[copyLength] = '\\0';
    
    return static_cast<int>(copyLength);
}

/**
 * Overloaded function for std::string input
 */
std::string handleInputToBuffer(const std::string& input) {
    char buffer[BUFFER_SIZE];
    handleInputToBuffer(input.c_str(), buffer);
    return std::string(buffer);
}

int main() {
    std::cout << "=== Secure Buffer Input Handler - C++ ===" << std::endl << std::endl;
    
    char buffer[BUFFER_SIZE];
    
    // Test case 1: Normal input
    std::string test1 = "Hello, World!";
    std::cout << "Test 1 - Normal input:" << std::endl;
    std::cout << "Input: " << test1 << std::endl;
    std::cout << "Output: " << handleInputToBuffer(test1) << std::endl;
    std::cout << std::endl;
    
    // Test case 2: Empty input
    std::string test2 = "";
    std::cout << "Test 2 - Empty input:" << std::endl;
    std::cout << "Input: (empty)" << std::endl;
    handleInputToBuffer(test2.c_str(), buffer);
    std::cout << "Output: " << buffer << std::endl;
    std::cout << std::endl;
    
    // Test case 3: Input at buffer limit
    std::string test3(99, 'A');
    std::cout << "Test 3 - Input at buffer limit (99 chars):" << std::endl;
    std::cout << "Input length: " << test3.length() << std::endl;
    int len3 = handleInputToBuffer(test3.c_str(), buffer);
    std::cout << "Output length: " << len3 << std::endl;
    std::cout << std::endl;
    
    // Test case 4: Input exceeding buffer size (security test)
    std::string test4(150, 'B');
    std::cout << "Test 4 - Input exceeding buffer (150 chars):" << std::endl;
    std::cout << "Input length: " << test4.length() << std::endl;
    int len4 = handleInputToBuffer(test4.c_str(), buffer);
    std::cout << "Output length: " << len4 << std::endl;
    std::cout << "Safely truncated: " << (len4 < static_cast<int>(test4.length()) ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test case 5: Null input
    std::cout << "Test 5 - Null input:" << std::endl;
    std::cout << "Input: nullptr" << std::endl;
    handleInputToBuffer(nullptr, buffer);
    std::cout << "Output: " << buffer << std::endl;
    std::cout << std::endl;
    
    return 0;
}
