
#include <iostream>
#include <cstring>
#include <string>
#include <algorithm>

#define BUFFER_SIZE 100

class Task104 {
public:
    /**
     * Safely handles user input into a fixed-size buffer
     * @param input The input string to be stored
     * @return The safely stored string (truncated if necessary)
     */
    static std::string handleInput(const std::string& input) {
        if (input.empty()) {
            return "";
        }
        
        // Ensure input doesn't exceed buffer size\n        if (input.length() > BUFFER_SIZE) {\n            return input.substr(0, BUFFER_SIZE);\n        }\n        \n        return input;\n    }\n    \n    /**\n     * Stores input into a character buffer safely\n     * @param input The input string\n     * @param buffer Pre-allocated buffer to store the input\n     */\n    static void storeInBuffer(const std::string& input, char* buffer) {\n        if (buffer == nullptr) {\n            return;\n        }\n        \n        // Initialize buffer with null characters\n        memset(buffer, 0, BUFFER_SIZE);\n        \n        if (input.empty()) {\n            return;\n        }\n        \n        size_t length = std::min(input.length(), static_cast<size_t>(BUFFER_SIZE - 1));\n        strncpy(buffer, input.c_str(), length);\n        buffer[length] = '\\0'; // Ensure null termination\n    }\n};\n\nint main() {\n    std::cout << "Testing Fixed-Size Buffer Input Handler\\n\\n";\n    \n    // Test case 1: Normal input\n    std::string test1 = "Hello, World!";\n    char buffer1[BUFFER_SIZE];\n    std::cout << "Test 1 - Normal input:\\n";\n    std::cout << "Input: " << test1 << "\\n";\n    std::cout << "Output: " << Task104::handleInput(test1) << "\\n";\n    Task104::storeInBuffer(test1, buffer1);\n    std::cout << "Buffer: " << buffer1 << "\\n\\n";\n    \n    // Test case 2: Empty input\n    std::string test2 = "";\n    std::cout << "Test 2 - Empty input:\\n";\n    std::cout << "Input: \\"\\"\\n";\n    std::cout << "Output: " << Task104::handleInput(test2) << "\\n\\n";\n    \n    // Test case 3: Input exceeding buffer size\n    std::string test3(150, 'A');\n    std::cout << "Test 3 - Input exceeding buffer size (150 chars):\\n";\n    std::cout << "Input length: " << test3.length() << "\\n";\n    std::string result3 = Task104::handleInput(test3);\n    std::cout << "Output length: " << result3.length() << "\\n";\n    std::cout << "Truncated: " << (test3.length() > result3.length() ? "true" : "false") << "\\n\\n";\n    \n    // Test case 4: Input at buffer boundary\n    std::string test4(100, 'B');
    std::cout << "Test 4 - Input at buffer boundary (100 chars):\\n";
    std::cout << "Input length: " << test4.length() << "\\n";
    std::string result4 = Task104::handleInput(test4);
    std::cout << "Output length: " << result4.length() << "\\n\\n";
    
    // Test case 5: Special characters
    std::string test5 = "Special: !@#$%^&*()";
    char buffer5[BUFFER_SIZE];
    std::cout << "Test 5 - Special characters:\\n";
    std::cout << "Input: " << test5 << "\\n";
    std::cout << "Output: " << Task104::handleInput(test5) << "\\n";
    Task104::storeInBuffer(test5, buffer5);
    std::cout << "Buffer: " << buffer5 << "\\n";
    
    return 0;
}
