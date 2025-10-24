#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Attempts to convert a string to an integer and prints the result or an error message.
 * It uses std::stoi, which can throw exceptions for invalid formats (std::invalid_argument)
 * or for values that are out of range (std::out_of_range). It also checks if the entire
 * string was consumed to disallow trailing characters.
 *
 * @param input The string to be converted.
 */
void convertStringToInteger(const std::string& input) {
    try {
        size_t pos;
        int result = std::stoi(input, &pos);

        // Check for trailing non-whitespace characters after the number
        for (size_t i = pos; i < input.length(); ++i) {
            if (!isspace(static_cast<unsigned char>(input[i]))) {
                std::cout << "Input: \"" << input << "\" -> Error: Not a valid integer (trailing characters)." << std::endl;
                return;
            }
        }

        std::cout << "Input: \"" << input << "\" -> Success: " << result << std::endl;
    } catch (const std::invalid_argument& ia) {
        std::cout << "Input: \"" << input << "\" -> Error: Not a valid integer (no conversion could be performed)." << std::endl;
    } catch (const std::out_of_range& oor) {
        std::cout << "Input: \"" << input << "\" -> Error: Number out of range for an integer." << std::endl;
    }
}

/**
 * Main function to run test cases for the string-to-integer conversion.
 */
int main() {
    std::vector<std::string> testCases = {
        "12345",             // Valid integer
        "  -99  ",           // Valid integer with whitespace
        "98 is a number",    // Invalid due to trailing characters
        "2147483648",        // Out of range (int max + 1)
        "abc"                // Invalid characters
    };

    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (const auto& test : testCases) {
        convertStringToInteger(test);
    }
    std::cout << "--- Test Cases Finished ---" << std::endl;

    return 0;
}