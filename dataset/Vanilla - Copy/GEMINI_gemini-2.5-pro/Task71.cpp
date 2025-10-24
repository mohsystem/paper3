#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Attempts to convert a string to an integer and prints the result or an error message.
 * @param s The string to be converted.
 */
void convertStringToInt(const std::string& s) {
    try {
        size_t pos;
        int number = std::stoi(s, &pos);
        // Check if the entire string was consumed by stoi
        if (pos != s.length()) {
             std::cout << "Input: \"" << s << "\" -> Error: Trailing characters found after number." << std::endl;
        } else {
             std::cout << "Input: \"" << s << "\" -> Converted to integer: " << number << std::endl;
        }
    } catch (const std::invalid_argument& ia) {
        std::cout << "Input: \"" << s << "\" -> Error: Invalid argument. No conversion could be performed." << std::endl;
    } catch (const std::out_of_range& oor) {
        std::cout << "Input: \"" << s << "\" -> Error: Out of range for an integer." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Handling command-line arguments if provided
    // argv[0] is the program name, arguments start at index 1
    if (argc > 1) {
        std::cout << "--- Processing Command-Line Arguments ---" << std::endl;
        for (int i = 1; i < argc; ++i) {
            convertStringToInt(argv[i]);
        }
        std::cout << "---------------------------------------" << std::endl;
    }

    // 5 hardcoded test cases
    std::cout << "\n--- Hardcoded Test Cases ---" << std::endl;
    std::vector<std::string> testCases = {"123", "-456", "abc", "2147483648", "789xyz"};
    for (const auto& test : testCases) {
        convertStringToInt(test);
    }
    std::cout << "--------------------------" << std::endl;

    return 0;
}