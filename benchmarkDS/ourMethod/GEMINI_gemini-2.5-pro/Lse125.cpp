#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Trims trailing whitespace from a string.
 * Whitespace characters are space, tab, newline, vertical tab, form feed, and carriage return.
 * @param s The input string.
 * @return A new string with trailing whitespace removed.
 */
std::string trimTrailingWhitespace(const std::string& s) {
    // find_last_not_of is a safe and standard way to find the first character
    // from the end that is not in the provided whitespace set.
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    
    if (std::string::npos == end) {
        // String is empty or contains only whitespace characters
        return "";
    }
    
    // Return the substring from the beginning to the last non-whitespace character
    return s.substr(0, end + 1);
}

void runTestCases() {
    std::cout << "\n--- Test Cases ---" << std::endl;
    const std::vector<std::string> testCases = {
        "hello world   ",
        "test\t\n ",
        "already_trimmed",
        "      ",
        ""
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        const std::string& testStr = testCases[i];
        std::string result = trimTrailingWhitespace(testStr);
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "  Original: \"" << testStr << "\"" << std::endl;
        std::cout << "  Trimmed:  \"" << result << "\"" << std::endl << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "--- Command Line Argument Processing ---" << std::endl;
    if (argc > 1) {
        // Process the first command line argument
        // argv[1] is guaranteed to be non-null if argc > 1
        std::string input(argv[1]);
        std::string trimmed = trimTrailingWhitespace(input);
        std::cout << "Original: \"" << input << "\"" << std::endl;
        std::cout << "Trimmed:  \"" << trimmed << "\"" << std::endl;
    } else {
        // Provide usage instructions if no argument is given
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program_name") << " \"<string to trim>\"" << std::endl;
    }

    runTestCases();

    return 0;
}