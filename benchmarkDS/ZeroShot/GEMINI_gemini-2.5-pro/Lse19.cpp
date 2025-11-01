#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

/**
 * @brief Parses a string to a long long, adds 1000, and returns the result.
 * This function is secure against invalid number formats and overflow.
 * @param inputStr The string representation of the number.
 * @return The resulting number after adding 1000.
 * @throws std::invalid_argument if the input is not a valid number.
 * @throws std::out_of_range if the input number is out of range or the
 *         addition would cause an overflow.
 */
long long addThousand(const std::string& inputStr) {
    long long num;
    size_t pos;

    try {
        // Secure: Use std::stoll for safe string-to-long-long conversion.
        num = std::stoll(inputStr, &pos);
        // Secure: Check if the entire string was consumed to reject inputs like "123foo".
        if (pos < inputStr.length()) {
            throw std::invalid_argument("Invalid characters found in input string.");
        }
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Input is not a valid number: \"" + inputStr + "\"");
    } catch (const std::out_of_range&) {
        throw std::out_of_range("Input number is out of range for a 64-bit integer.");
    }

    // Secure: Check for potential overflow before performing the addition.
    if (num > std::numeric_limits<long long>::max() - 1000) {
        throw std::out_of_range("Addition would cause an overflow.");
    }
    
    return num + 1000;
}

void runTestCase(int caseNum, const std::string& input) {
    std::cout << "Test Case " << caseNum << ": Input = \"" << input << "\"" << std::endl;
    try {
        long long result = addThousand(input);
        std::cout << "  -> Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  -> Error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Part 1: Process command-line argument if provided.
    if (argc == 2) {
        std::cout << "Processing command-line argument: " << argv[1] << std::endl;
        try {
            long long result = addThousand(argv[1]);
            std::cout << "Result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
        std::cout << "\n-----------------------------------------\n" << std::endl;
    } else if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " <number>" << std::endl;
        std::cerr << "Ignoring extra arguments and running test cases." << std::endl;
        std::cout << "\n-----------------------------------------\n" << std::endl;
    }

    // Part 2: Run 5 hardcoded test cases to demonstrate functionality.
    std::cout << "Running 5 test cases..." << std::endl;
    runTestCase(1, "50");
    runTestCase(2, "-50");
    runTestCase(3, "0");
    runTestCase(4, "50 bottles");
    runTestCase(5, "9223372036854775807"); // LLONG_MAX, will cause overflow
    
    return 0;
}