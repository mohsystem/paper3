#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio> // For snprintf

/**
 * @brief Safely copies a string into a fixed-size character buffer.
 * 
 * This function uses snprintf, which is a secure way to prevent buffer overflows.
 * It guarantees that the destination buffer will not be overflowed and will be
 * null-terminated.
 *
 * @param buffer The destination character array (the buffer).
 * @param bufferSize The total size of the destination buffer.
 * @param input The source string to copy from.
 */
void handleInput(char* buffer, size_t bufferSize, const std::string& input) {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    // snprintf is a safe function that writes at most bufferSize - 1 characters
    // from the input and always appends a null terminator.
    snprintf(buffer, bufferSize, "%s", input.c_str());
}

int main() {
    const size_t BUFFER_SIZE = 20;

    std::vector<std::string> testCases = {
        "",                                         // Empty string
        "short",                                    // Shorter than buffer
        "This is exactly 19!",                      // Exactly buffer size - 1
        "This input is too long for the buffer",    // Longer than buffer
        "This is a very very very very very very very long input string that will surely be truncated" // Very long string
    };

    std::cout << "C++ Test Cases (Buffer Size: " << BUFFER_SIZE << ")" << std::endl;
    std::cout << "------------------------------------" << std::endl;

    for (int i = 0; i < testCases.size(); ++i) {
        char buffer[BUFFER_SIZE];
        
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "Input: \"" << testCases[i] << "\"" << std::endl;
        
        handleInput(buffer, BUFFER_SIZE, testCases[i]);
        
        std::cout << "Buffer Content: \"" << buffer << "\"" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}