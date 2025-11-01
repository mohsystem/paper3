#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <limits>

/**
 * @brief Parses a string to a long long, adds 1000, and checks for overflow.
 * @param inputStr The string to parse.
 * @return The result of the addition.
 * @throws std::invalid_argument if the string is not a valid number.
 * @throws std::out_of_range if the number is too large for a long long.
 * @throws std::overflow_error if adding 1000 would cause an overflow.
 */
long long processValue(const std::string& inputStr) {
    long long value;
    size_t pos;
    try {
        value = std::stoll(inputStr, &pos);
        // Ensure the entire string was consumed
        if (pos != inputStr.length()) {
            throw std::invalid_argument("Input contains non-numeric characters.");
        }
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Input is not a valid integer.");
    } catch (const std::out_of_range&) {
        throw std::out_of_range("Input is out of range for a 64-bit integer.");
    }

    if (value > std::numeric_limits<long long>::max() - 1000) {
        throw std::overflow_error("Adding 1000 would cause an overflow.");
    }

    return value + 1000;
}

/**
 * @brief Runs a set of test cases against the processValue function.
 */
void runTests() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    std::vector<std::string> test_cases = {
        "10",                           // Valid positive
        "0",                            // Valid zero
        "-50",                          // Valid negative
        "not a number",                 // Invalid format
        std::to_string(std::numeric_limits<long long>::max()) // Overflow on add
    };

    for (const auto& test : test_cases) {
        std::cout << "Input: \"" << test << "\" -> ";
        try {
            long long result = processValue(test);
            std::cout << "Output: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}

/**
 * @brief Main function.
 * If one argument is provided, it's processed.
 * If no arguments, tests are run.
 * Otherwise, usage is shown.
 */
int main(int argc, char* argv[]) {
    if (argc == 2) {
        try {
            long long result = processValue(argv[1]);
            std::cout << result << std::endl;
            return 0; // Success
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1; // Failure
        }
    } else if (argc == 1) {
        runTests();
        return 0; // Tests ran successfully
    } else {
        std::cerr << "Usage: " << argv[0] << " <integer>" << std::endl;
        return 1; // Failure
    }
}