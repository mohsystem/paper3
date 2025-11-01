#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

/**
 * @brief Parses a string, adds 1000, and returns the result.
 * 
 * This function includes security checks for input format, range, and potential overflow.
 * @param inputStr The string representation of the number.
 * @return The number after adding 1000.
 * @throws std::invalid_argument if the input string is not a valid number.
 * @throws std::out_of_range if the input number is out of the valid range.
 * @throws std::overflow_error if adding 1000 would cause an overflow.
 */
long long addThousand(const std::string& inputStr) {
    long long number;
    size_t pos;
    try {
        number = std::stoll(inputStr, &pos);
        // Secure check: ensure the entire string was consumed.
        // Prevents parsing "123abc" as 123.
        if (pos != inputStr.length()) {
            throw std::invalid_argument("Error: Input contains non-numeric characters.");
        }
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Error: Input is not a valid integer.");
    } catch (const std::out_of_range&) {
        throw std::out_of_range("Error: Input number is out of range for a 64-bit integer.");
    }

    // Secure check for potential overflow before performing addition.
    if (number > std::numeric_limits<long long>::max() - 1000) {
        throw std::overflow_error("Error: Adding 1000 would cause an overflow.");
    }

    return number + 1000;
}

/**
 * @brief Main entry point. Reads a single command-line argument and processes it.
 * Includes 5 test cases in the comments demonstrating how to run the program.
 */
int main(int argc, char* argv[]) {
    /*
     * How to compile and run:
     * 1. Compile: g++ -std=c++11 -o Lse21 <source_file>.cpp
     * 2. Run:     ./Lse21 <value>
     *
     * Test Cases:
     * 1. Normal positive value: ./Lse21 50
     *    Expected output: 1050
     * 2. Normal negative value: ./Lse21 -2000
     *    Expected output: -1000
     * 3. Zero value:            ./Lse21 0
     *    Expected output: 1000
     * 4. Invalid input (text):  ./Lse21 "hello"
     *    Expected output: Error: Input is not a valid integer.
     * 5. Overflow value:        ./Lse21 9223372036854775807
     *    Expected output: Error: Adding 1000 would cause an overflow.
    */
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <integer_value>" << std::endl;
        return 1; // Return non-zero for error
    }

    try {
        long long result = addThousand(argv[1]);
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}