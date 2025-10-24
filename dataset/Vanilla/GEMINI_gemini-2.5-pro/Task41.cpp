#include <iostream>
#include <string> // Use std::string for safety
#include <vector>

/**
 * @brief Processes the input string using std::string.
 *
 * Using std::string is the idiomatic and safe C++ way to handle strings. It manages
 * its own memory, automatically resizing as needed, thus preventing the buffer
 * overflow vulnerabilities common with C-style char arrays.
 *
 * @param input A constant reference to a std::string to be processed.
 */
void processString(const std::string& input) {
    std::cout << "Processing input: \"" << input << "\"" << std::endl;
    std::cout << "Length: " << input.length() << std::endl;
    // In a real application, further safe processing would occur here.
    std::cout << "Processing complete." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

int main() {
    std::cout << "--- C++ Test Cases (Using std::string for Safety) ---" << std::endl;

    std::vector<std::string> testCases;

    // Test Case 1: A typical short string
    testCases.push_back("Hello, World!");

    // Test Case 2: An empty string
    testCases.push_back("");

    // Test Case 3: A long string that would overflow a small fixed buffer
    testCases.push_back("This is a very long string designed to test the dynamic memory allocation of std::string, which prevents buffer overflows automatically.");

    // Test Case 4: A string with special characters
    testCases.push_back("!@#$%^&*()_+-={}[]|;'<>,.?/`~");

    // Test Case 5: A string containing null characters
    testCases.push_back(std::string("A string with an embedded\0null char.", 37));
    
    for (const auto& test : testCases) {
        processString(test);
    }

    return 0;
}