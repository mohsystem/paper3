#include <iostream>
#include <string>
#include <limits>

/**
 * Reads a line from standard input into the provided string reference.
 * The string is truncated to 1023 characters if it is longer.
 * The newline character is consumed from the input stream but not stored.
 *
 * @param result The std::string to store the read line.
 * @return The number of characters read and stored in the result string.
 */
int scan(std::string& result) {
    if (std::getline(std::cin, result)) {
        if (result.length() > 1023) {
            result.resize(1023);
        }
    } else {
        // Handle EOF or read error by clearing the string
        result.clear();
    }
    return result.length();
}

int main() {
    std::cout << "Running 5 test cases. Please provide input for each." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        std::cout << "Enter a string: ";
        
        std::string result_str;
        int chars_read = scan(result_str);
        
        std::cout << "String read: \"" << result_str << "\"" << std::endl;
        std::cout << "Number of characters read: " << chars_read << std::endl;
    }
    return 0;
}