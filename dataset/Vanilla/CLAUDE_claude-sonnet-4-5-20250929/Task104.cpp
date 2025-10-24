
#include <iostream>
#include <cstring>
#include <string>

#define BUFFER_SIZE 100

void handleUserInput(const char* input, char* buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    
    if (input == nullptr) {
        return;
    }
    
    size_t length = strlen(input);
    size_t copyLength = (length < BUFFER_SIZE) ? length : BUFFER_SIZE - 1;
    
    strncpy(buffer, input, copyLength);
    buffer[copyLength] = '\\0';
}

std::string handleUserInput(const std::string& input) {
    if (input.length() > BUFFER_SIZE) {
        return input.substr(0, BUFFER_SIZE);
    }
    return input;
}

int main() {
    char buffer[BUFFER_SIZE];
    
    std::cout << "Test Case 1: Normal input" << std::endl;
    std::string input1 = "Hello, World!";
    std::string result1 = handleUserInput(input1);
    std::cout << "Input: " << input1 << std::endl;
    std::cout << "Output: " << result1 << std::endl;
    std::cout << "Length: " << result1.length() << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 2: Empty input" << std::endl;
    std::string input2 = "";
    std::string result2 = handleUserInput(input2);
    std::cout << "Input: (empty)" << std::endl;
    std::cout << "Output: " << result2 << std::endl;
    std::cout << "Length: " << result2.length() << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 3: Input exceeding buffer size" << std::endl;
    std::string input3(150, 'A');
    std::string result3 = handleUserInput(input3);
    std::cout << "Input length: " << input3.length() << std::endl;
    std::cout << "Output length: " << result3.length() << std::endl;
    std::cout << "Truncated: " << (result3.length() == BUFFER_SIZE ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Input at buffer boundary" << std::endl;
    std::string input4(100, 'B');
    std::string result4 = handleUserInput(input4);
    std::cout << "Input length: " << input4.length() << std::endl;
    std::cout << "Output length: " << result4.length() << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: Using C-style buffer" << std::endl;
    const char* input5 = "Testing C-style strings";
    handleUserInput(input5, buffer);
    std::cout << "Input: " << input5 << std::endl;
    std::cout << "Output: " << buffer << std::endl;
    std::cout << "Length: " << strlen(buffer) << std::endl;
    
    return 0;
}
