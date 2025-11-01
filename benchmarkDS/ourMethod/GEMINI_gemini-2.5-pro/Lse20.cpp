#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

const int OFFSET = 1000;

/**
 * @brief Adds a constant offset to an integer value, checking for overflow.
 * 
 * @param value The integer to add the offset to.
 * @return The result of the addition.
 * @throw std::overflow_error if the addition would cause an integer overflow.
 */
int add_offset(int value) {
    if (value > 0 && value > std::numeric_limits<int>::max() - OFFSET) {
        throw std::overflow_error("Integer overflow detected on addition.");
    }
    // No underflow check is necessary when adding a positive constant.
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <integer_value>" << std::endl;
        return 1;
    }

    try {
        std::string input_str(argv[1]);
        size_t pos;
        // Use stoll to parse into a larger type to safely check range later.
        long long long_val = std::stoll(input_str, &pos);

        // Check if the entire string was consumed.
        if (pos != input_str.length()) {
            std::cerr << "Error: Trailing non-numeric characters in input." << std::endl;
            return 1;
        }

        // Check if the value fits within the range of a standard int.
        if (long_val > std::numeric_limits<int>::max() || long_val < std::numeric_limits<int>::min()) {
             std::cerr << "Error: Input value out of range for an integer." << std::endl;
             return 1;
        }

        int value = static_cast<int>(long_val);
        int result = add_offset(value);
        std::cout << "Result: " << result << std::endl;

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Input is not a valid integer." << std::endl;
        return 1;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Input value is out of range." << std::endl;
        return 1;
    } catch (const std::overflow_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    /*
    To run the test cases, compile the code first:
    g++ -o lse20 lse20.cpp -std=c++11 -Wall -Werror

    Then run from the command line:

    Test Case 1: Positive number
    Command: ./lse20 50
    Expected Output: Result: 1050

    Test Case 2: Negative number
    Command: ./lse20 -50
    Expected Output: Result: 950

    Test Case 3: Zero
    Command: ./lse20 0
    Expected Output: Result: 1000

    Test Case 4: Invalid input
    Command: ./lse20 abc
    Expected Output: Error: Input is not a valid integer.

    Test Case 5: Overflow
    Command: ./lse20 2147483647
    Expected Output: Error: Integer overflow detected on addition.
    */

    return 0;
}