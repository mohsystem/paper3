#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

/**
 * Processes a string using std::string, which is C++'s safe way
 * to handle strings. It automatically manages its own memory,
 * preventing buffer overflows.
 *
 * @param input The const std::string& to be processed.
 */
void processString(const std::string& input) {
    std::cout << "Processing input: \"" << input << "\"" << std::endl;
    // std::string handles memory automatically, making operations like this safe.
    std::string processedString = "Processed: ";
    processedString += input;
    std::transform(processedString.begin(), processedString.end(), processedString.begin(),
                   [](unsigned char c){ return std::toupper(c); });

    std::cout << "Output: \"" << processedString << "\"" << std::endl;
    std::cout << "------------------------------------" << std::endl;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    std::vector<std::string> testCases = {
        // Test Case 1: A normal, short string
        "Hello World",
        // Test Case 2: An empty string
        "",
        // Test Case 3: A string with various special characters
        "!@#$%^&*()_+-=[]{}|;':,./<>?",
        // Test Case 4: A very long string
        "This is a very long string that in a language like C could potentially cause a buffer overflow if not handled carefully. In C++, std::string will simply resize to accommodate the data.",
        // Test Case 5: A string with Unicode characters (note: toupper is locale-dependent)
        "Secure Code (安全なコード)"
    };

    for (const auto& test : testCases) {
        processString(test);
    }

    return 0;
}