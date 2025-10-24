#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <algorithm> // For std::isspace

/**
 * Attempts to convert a string to an integer.
 * This function is secure against invalid input by using a try-catch block
 * with std::stoi, which throws exceptions for invalid arguments or
 * out-of-range values. It returns an std::optional to safely indicate
 * success or failure without relying on magic numbers.
 *
 * @param input The string to be converted.
 * @return An std::optional<int> containing the integer if successful,
 *         otherwise std::nullopt.
 */
std::optional<int> convertToInteger(const std::string& input) {
    try {
        size_t pos; // To check if the entire string was consumed
        int result = std::stoi(input, &pos);

        // Check for trailing characters that are not whitespace
        for (size_t i = pos; i < input.length(); ++i) {
            if (!std::isspace(static_cast<unsigned char>(input[i]))) {
                return std::nullopt; // Found trailing non-whitespace chars
            }
        }
        return result;
    } catch (const std::invalid_argument& e) {
        // Conversion could not be performed (e.g., "abc")
        return std::nullopt;
    } catch (const std::out_of_range& e) {
        // The converted value would be out of the range of an int
        return std::nullopt;
    }
}

int main() {
    std::vector<std::string> testCases = {
        "123",        // Valid positive integer
        "-456",       // Valid negative integer
        "  789  ",    // Valid integer with whitespace
        "abc",        // Invalid non-numeric string
        "2147483648", // Out of range for a 32-bit signed integer
        "123a"        // Trailing non-numeric characters
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\" -> ";
        auto result = convertToInteger(testCase);
        if (result) { // or result.has_value()
            std::cout << "Success, value: " << result.value() << std::endl;
        } else {
            std::cout << "Failed to convert." << std::endl;
        }
    }
    return 0;
}