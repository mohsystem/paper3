#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Attempts to convert a given string to an integer and prints the outcome.
 * @param input The string to be converted.
 */
void attemptConversion(const std::string& input) {
    try {
        size_t processed_chars;
        int number = std::stoi(input, &processed_chars);

        // After conversion, check if there are any non-whitespace characters left.
        // std::stoi will parse "12a34" as 12, so we need this extra check.
        for (size_t i = processed_chars; i < input.length(); ++i) {
            if (!isspace(static_cast<unsigned char>(input[i]))) {
                // Found a non-whitespace character after the number.
                throw std::invalid_argument("Trailing characters found.");
            }
        }
        std::cout << "Input: \"" << input << "\" -> Success! Converted to integer: " << number << std::endl;
    } catch (const std::invalid_argument& ia) {
        // This catches strings that don't start with a number (like "abc")
        // or have trailing characters as per our check above.
        std::cout << "Input: \"" << input << "\" -> Failed. Not a valid integer." << std::endl;
    } catch (const std::out_of_range& oor) {
        // This catches numbers that are too large or small to fit in an int.
        std::cout << "Input: \"" << input << "\" -> Failed. Number is out of range." << std::endl;
    }
}

int main() {
    // Vector of 5 test cases to demonstrate the functionality.
    std::vector<std::string> testCases = {
        "123",      // Valid positive integer
        "-45",      // Valid negative integer
        "  99  ",   // Valid integer with whitespace
        "abc",      // Invalid string
        "12a34"     // Invalid string with mixed characters
    };

    std::cout << "--- C++: Testing Integer Conversion ---" << std::endl;
    for (const auto& test : testCases) {
        attemptConversion(test);
    }
    return 0;
}