#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <limits>

/**
 * Attempts to convert a string to an integer.
 * Handles invalid formats and values outside the range of a standard int.
 *
 * @param input The string to be converted.
 * @param outValue A reference to an integer where the result will be stored.
 * @return true if conversion is successful, false otherwise.
 */
bool convertStringToInt(const std::string& input, int& outValue) {
    try {
        // Use std::stoll to parse as a long long first to detect overflow
        // against the int type before it happens.
        long long long_val = std::stoll(input);
        
        // Check if the parsed value is within the range of an int.
        if (long_val < std::numeric_limits<int>::min() || long_val > std::numeric_limits<int>::max()) {
            return false; // Out of range for an int
        }
        
        outValue = static_cast<int>(long_val);
        return true;
    } catch (const std::invalid_argument& ia) {
        // This exception is thrown if no conversion could be performed (e.g., "abc").
        return false;
    } catch (const std::out_of_range& oor) {
        // This exception is thrown if the value is out of range for a long long.
        return false;
    }
}

int main(int argc, char* argv[]) {
    // Section 1: Process command-line argument if provided
    if (argc > 1) {
        std::cout << "--- Processing Command-Line Argument ---" << std::endl;
        std::string cliInput(argv[1]);
        std::cout << "Input: \"" << cliInput << "\"" << std::endl;
        int result;
        if (convertStringToInt(cliInput, result)) {
            std::cout << "Successfully converted to: " << result << std::endl;
        } else {
            std::cout << "Error: Conversion failed. Input is not a valid integer." << std::endl;
        }
        std::cout << "--------------------------------------" << std::endl << std::endl;
    } else {
        std::cout << "--- No Command-Line Argument Provided ---" << std::endl << std::endl;
    }

    // Section 2: Run 5 hardcoded test cases
    std::cout << "--- Running Hardcoded Test Cases ---" << std::endl;
    std::vector<std::string> testCases = {
        "789",          // Valid integer
        "-123",         // Valid negative integer
        "cpp",          // Invalid format
        "2147483648",   // Out of range (max int + 1)
        "  -50  "       // Valid with whitespace
    };

    for (int i = 0; i < testCases.size(); ++i) {
        const std::string& testInput = testCases[i];
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input: \"" << testInput << "\"" << std::endl;
        int result;
        if (convertStringToInt(testInput, result)) {
            std::cout << "Successfully converted to: " << result << std::endl;
        } else {
            std::cout << "Error: Conversion failed. Input is not a valid integer." << std::endl;
        }
        std::cout << std::endl; // For spacing
    }
    std::cout << "------------------------------------" << std::endl;

    return 0;
}