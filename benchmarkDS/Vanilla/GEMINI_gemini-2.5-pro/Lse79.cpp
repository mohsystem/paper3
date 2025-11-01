#include <iostream>
#include <string>
#include <vector>
#include <cstring> // For memset and strncpy

/**
 * Allocates two 1024-byte buffers and copies up to 100 characters
 * from the input strings into them.
 * @param input1 The first string to copy.
 * @param input2 The second string to copy.
 */
void processStrings(const std::string& input1, const std::string& input2) {
    // Allocate first 1024-byte buffer
    char* buf1 = new char[1024];
    // Initialize buffer with null characters
    std::memset(buf1, 0, 1024);

    // Copy up to 100 characters from input1 into buf1.
    // strncpy is used for this fixed-size copy.
    // If input1 has >= 100 chars, it won't be null-terminated by strncpy,
    // but our memset ensures the rest of the buffer is null.
    strncpy(buf1, input1.c_str(), 100);
    if(input1.length() >= 100) {
        buf1[100] = '\0'; // Manually terminate for safety
    }

    // Allocate second 1024-byte buffer
    char* buf2 = new char[1024];
    std::memset(buf2, 0, 1024);
    
    // Copy up to 100 characters from input2 into buf2
    strncpy(buf2, input2.c_str(), 100);
    if(input2.length() >= 100) {
        buf2[100] = '\0'; // Manually terminate for safety
    }

    std::cout << "Buffer 1 content: \"" << buf1 << "\"" << std::endl;
    std::cout << "Buffer 2 content: \"" << buf2 << "\"" << std::endl;

    // Clean up dynamically allocated memory
    delete[] buf1;
    delete[] buf2;
}

int main() {
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"Hello World", "This is a test"},
        {"A short string", "A very very very very very very very very very very very very very very very very very very very long string that will be truncated."},
        {"", "An empty string for the first input."},
        {"Another test case with numbers 12345.", ""},
        {"Last test case! Using special characters: !@#$%^&*()", "And another one."}
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input 1: \"" << testCases[i].first << "\"" << std::endl;
        std::cout << "Input 2: \"" << testCases[i].second << "\"" << std::endl;
        processStrings(testCases[i].first, testCases[i].second);
        std::cout << "---------------------\n" << std::endl;
    }

    return 0;
}