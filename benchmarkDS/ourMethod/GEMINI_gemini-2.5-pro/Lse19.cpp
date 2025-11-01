#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>
#include <vector>

/**
 * Parses a string to a long long, adds 1000 to it.
 * Throws exceptions for invalid input or overflow.
 *
 * @param inputStr The string to parse.
 * @return The result of the addition.
 * @throws std::invalid_argument if the input is not a valid number.
 * @throws std::out_of_range if the input number is out of range for long long
 *                           or if the addition would cause an overflow.
 */
long long addThousand(const std::string& inputStr) {
    size_t pos;
    long long value;
    
    try {
        value = std::stoll(inputStr, &pos);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Error: Input is not a number.");
    } catch (const std::out_of_range&) {
        throw std::out_of_range("Error: Input number is out of range.");
    }

    // Ensure the entire string was consumed
    if (pos != inputStr.length()) {
        throw std::invalid_argument("Error: Input contains non-numeric characters.");
    }

    // Check for potential overflow before adding
    if (value > std::numeric_limits<long long>::max() - 1000) {
        throw std::out_of_range("Error: Input value is too large and would cause an overflow.");
    }

    return value + 1000;
}

/**
 * Runs a series of test cases for the addThousand function.
 */
void runTests() {
    std::cout << "\n--- Running Test Cases ---" << std::endl;
    std::vector<std::string> testCases = {
        "10", 
        "0", 
        "-50",
        "9223372036854775807", // LLONG_MAX, will cause overflow
        "not a number",
        "123foo",
        "9223372036854774807"  // LLONG_MAX - 1000, valid
    };

    for (const auto& test : testCases) {
        std::cout << "Test case: \"" << test << "\" -> ";
        try {
            long long result = addThousand(test);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << e.what() << " (Handled as expected)" << std::endl;
        }
    }
    std::cout << "--- Test Cases Finished ---" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        std::cout << "Processing command line argument: " << argv[1] << std::endl;
        try {
            long long result = addThousand(argv[1]);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " <number>" << std::endl;
        std::cout << "No command line argument provided. Running test cases instead." << std::endl;
    }

    // Running 5+ test cases as requested.
    runTests();
    
    return EXIT_SUCCESS;
}