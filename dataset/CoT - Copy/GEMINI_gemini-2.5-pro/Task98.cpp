#include <iostream>
#include <string>
#include <stdexcept>

/**
 * Attempts to convert a string to an integer and prints the result or an error message.
 *
 * @param input The string to be converted.
 */
void convertAndPrint(const std::string& input) {
    if (input.empty()) {
        std::cout << "Error: Invalid input. String is empty." << std::endl;
        return;
    }
    try {
        size_t pos;
        int number = std::stoi(input, &pos);

        // Ensure the entire string was consumed
        if (pos != input.length()) {
            std::cout << "Error: Invalid input. Trailing characters found in '" << input << "'." << std::endl;
        } else {
            std::cout << "Successfully converted to integer: " << number << std::endl;
        }
    } catch (const std::invalid_argument& ia) {
        std::cout << "Error: Invalid argument. Cannot convert '" << input << "' to an integer." << std::endl;
    } catch (const std::out_of_range& oor) {
        std::cout << "Error: Out of range. The value '" << input << "' is too large or too small for an integer." << std::endl;
    }
}

int main() {
    std::string testCases[] = {"123", "-456", "abc", "2147483648", "123a"};

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (const auto& test : testCases) {
        std::cout << "Input: \"" << test << "\" -> ";
        convertAndPrint(test);
    }
    std::cout << "--- End of Test Cases ---" << std::endl;
    return 0;
}