#include <iostream>
#include <string>
#include <vector>
#include <cctype> // For isspace

/**
 * Trims trailing whitespace from a std::string in-place.
 *
 * @param str The string to trim.
 */
void trimTrailingWhitespace(std::string &str) {
    // The for loop trims the trailing whitespace in the string.
    // The second for loop trims the newlines.
    // (Combined into one loop for efficiency as isspace handles both.)
    while (!str.empty() && isspace(static_cast<unsigned char>(str.back()))) {
        str.pop_back();
    }
    // std::string handles its own termination, so no manual null termination is needed.
}

void run_test_cases() {
    std::cout << "\n--- Running Test Cases ---" << std::endl;
    
    std::vector<std::string> tests = {
        "hello world  ",
        "test\t\n \r",
        "already_clean",
        "   \t\n",
        ""
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string test_str = tests[i];
        std::cout << "--- Test Case " << i + 1 << " ---" << std::endl;
        std::cout << "Original: \"" << test_str << "\"" << std::endl;
        trimTrailingWhitespace(test_str);
        std::cout << "Trimmed : \"" << test_str << "\"\n" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " \"<string to trim>\"" << std::endl;
        std::cerr << "Running test cases instead." << std::endl;
        run_test_cases();
        return 1;
    }

    // Check the argument to make sure that it is a string (argv elements are C-style strings)
    std::string input(argv[1]);
    std::cout << "Original: \"" << input << "\"" << std::endl;
    trimTrailingWhitespace(input);
    std::cout << "Trimmed : \"" << input << "\"" << std::endl;

    return 0;
}