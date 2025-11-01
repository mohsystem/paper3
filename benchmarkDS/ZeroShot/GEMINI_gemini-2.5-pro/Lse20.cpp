#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <limits>

const long long OFFSET = 1000;

/**
 * @brief Adds a constant offset to a given value, checking for overflow.
 * @param value The input number.
 * @return The value with the offset added.
 * @throw std::overflow_error if the addition would cause an overflow.
 */
long long addOffset(long long value) {
    // Secure: Check for potential overflow before performing the addition.
    if (value > std::numeric_limits<long long>::max() - OFFSET) {
        throw std::overflow_error("Overflow detected on addition");
    }
    return value + OFFSET;
}

/**
 * @brief Helper function to parse input, run the logic, and print results for a test case.
 * @param testStr The string input to test.
 */
void runTest(const std::string& testStr) {
    std::cout << "Test Case: \"" << testStr << "\" -> ";
    try {
        size_t pos;
        long long inputValue = std::stoll(testStr, &pos);

        // Secure: Check if the entire string was consumed during parsing.
        // This prevents accepting inputs like "123xyz".
        if (pos != testStr.length()) {
             throw std::invalid_argument("Invalid characters in number");
        }

        long long result = addOffset(inputValue);
        std::cout << "Result: " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: Invalid number format." << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Error: Number is out of range for a long long." << std::endl;
    } catch (const std::overflow_error& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Part 1: Handle actual command line input
    if (argc == 2) {
        std::cout << "--- Processing Command Line Input ---" << std::endl;
        runTest(argv[1]);
        std::cout << "-------------------------------------\n" << std::endl;
    } else if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " <single integer value>" << std::endl;
    }

    // Part 2: Run 5+ built-in test cases as requested
    std::cout << "--- Running Built-in Test Cases ---" << std::endl;
    std::vector<std::string> testCases = {
        "123",
        "0",
        "-50",
        "9223372036854775807", // LLONG_MAX, will cause overflow
        "abc",
        "123xyz",
        "-9223372036854775808" // LLONG_MIN
    };
    for (const auto& test : testCases) {
        runTest(test);
    }
    std::cout << "-----------------------------------" << std::endl;

    return 0;
}