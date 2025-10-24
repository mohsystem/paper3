#include <iostream>
#include <string>
#include <vector>

/**
 * Processes a string by printing it and its length.
 * Using std::string is the standard C++ way to safely handle strings.
 * It automatically manages memory, preventing buffer overflows.
 * @param input The string to process, passed as a constant reference for efficiency.
 */
void processString(const std::string& input) {
    std::cout << "Processed String: " << input << std::endl;
    std::cout << "Length: " << input.length() << std::endl;
    std::cout << "--------------------" << std::endl;
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    std::vector<std::string> testCases = {
        // Test Case 1: Regular string
        "Hello, World!",
        // Test Case 2: Another typical string
        "This is a test string.",
        // Test Case 3: Empty string
        "",
        // Test Case 4: A very long string
        "This is a very long string to demonstrate that std::string handles memory dynamically, so there is no fixed buffer to overflow unlike in C.",
        // Test Case 5: String with special characters
        "!@#$%^&*()_+-=[]{}|;:'\",.<>?`~"
    };

    for (const auto& test : testCases) {
        processString(test);
    }

    std::cout << "--- End of Test Cases ---" << std::endl;
    return 0;
}