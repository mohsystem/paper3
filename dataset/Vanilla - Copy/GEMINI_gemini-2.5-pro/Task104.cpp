#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

void handleInput(const std::string& input, char* buffer, size_t bufferSize) {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    // Copy at most bufferSize - 1 characters to leave space for the null terminator.
    size_t len_to_copy = input.copy(buffer, bufferSize - 1);
    // Add the null terminator.
    buffer[len_to_copy] = '\0';
}

int main() {
    const size_t BUFFER_SIZE = 16;
    std::vector<std::string> testCases = {
        "Hello World",
        "This is a long string that will be truncated",
        "123456789012345", // Exactly 15 characters to fit with '\0'
        "",
        "Another long test for truncation"
    };
    
    for (const auto& test : testCases) {
        char buffer[BUFFER_SIZE];
        handleInput(test, buffer, BUFFER_SIZE);
        std::cout << "Input:  \"" << test << "\"" << std::endl;
        std::cout << "Output: \"" << buffer << "\"\n" << std::endl;
    }
    return 0;
}