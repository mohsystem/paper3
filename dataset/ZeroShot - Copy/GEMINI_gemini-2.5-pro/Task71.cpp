#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <climits> // For INT_MIN, INT_MAX

/**
 * Securely converts a string to an integer.
 * Handles invalid formats and integer overflow/underflow using std::stoi.
 *
 * @param input The string to convert.
 * @return An optional<int> containing the integer if conversion is successful, otherwise an empty optional.
 */
std::optional<int> convertStringToInt(const std::string& input) {
    try {
        size_t pos;
        int value = std::stoi(input, &pos);

        // Check if the entire string was consumed.
        // This prevents accepting inputs like "123xyz".
        if (pos != input.length()) {
            std::cerr << "Error: Input string '" << input << "' contains non-numeric characters." << std::endl;
            return std::nullopt;
        }
        return value;
    } catch (const std::invalid_argument& e) {
        // This exception is thrown if no conversion could be performed.
        std::cerr << "Error: '" << input << "' is not a valid integer format." << std::endl;
        return std::nullopt;
    } catch (const std::out_of_range& e) {
        // This exception is thrown if the converted value is out of range of int.
        std::cerr << "Error: '" << input << "' is out of the integer range." << std::endl;
        return std::nullopt;
    }
}

int main(int argc, char* argv[]) {
    // Handle command-line argument if provided
    if (argc > 1) {
        std::cout << "--- Command-Line Input ---" << std::endl;
        std::string cliInput(argv[1]);
        std::cout << "Input: \"" << cliInput << "\"" << std::endl;
        auto result = convertStringToInt(cliInput);
        if (result) {
            std::cout << "Successfully converted to: " << *result << std::endl;
        } else {
            std::cout << "Conversion failed." << std::endl;
        }
        std::cout << "--------------------------" << std::endl << std::endl;
    } else {
        std::cout << "Usage: ./<executable_name> <string_to_convert>" << std::endl;
        std::cout << "Running built-in test cases as no command-line argument was provided." << std::endl << std::endl;
    }

    // Run 5 test cases
    std::cout << "--- Built-in Test Cases ---" << std::endl;
    std::vector<std::string> testCases = {
        "123",          // 1. Valid positive integer
        "-456",         // 2. Valid negative integer
        "hello",        // 3. Invalid format
        "2147483648",   // 4. Integer overflow
        "-2147483649"   // 5. Integer underflow
    };

    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\"" << std::endl;
        auto result = convertStringToInt(testCase);
        if (result) {
            std::cout << "Successfully converted to: " << *result << std::endl;
        } else {
            std::cout << "Conversion failed." << std::endl;
        }
        std::cout << std::endl; // for spacing
    }
    std::cout << "---------------------------" << std::endl;

    return 0;
}