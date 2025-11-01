#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// Use long long for a wider range of values to prevent overflow
const long long OFFSET = 1000;

/**
 * Adds a constant offset to the given value.
 *
 * @param value The input number.
 * @return The value with the offset added.
 */
long long addOffset(long long value) {
    return value + OFFSET;
}

void process_input(const std::string& input_str) {
    try {
        // Use stoll for string to long long conversion, which throws exceptions on failure
        long long input_value = std::stoll(input_str);
        long long result = addOffset(input_value);
        std::cout << "Input: " << input_value << ", Result: " << result << std::endl;
    } catch (const std::invalid_argument& ia) {
        std::cerr << "Error: Input '" << input_str << "' is not a valid number." << std::endl;
    } catch (const std::out_of_range& oor) {
        std::cerr << "Error: Input '" << input_str << "' is out of range for a long long." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // We can use the first command-line argument if provided.
    if (argc > 1) {
        std::cout << "Processing command-line argument: " << argv[1] << std::endl;
        process_input(argv[1]);
        std::cout << "\n--- Running Internal Test Cases ---" << std::endl;
    }
    
    // 5 internal test cases for demonstration
    std::vector<std::string> test_cases = {
        "75",           // Basic positive number
        "-2000",        // Basic negative number
        "0",            // Zero
        "9000000000000000000", // A very large number
        "not-a-number"  // Invalid input
    };

    for (const auto& test : test_cases) {
        std::cout << "Processing test case: " << test << std::endl;
        process_input(test);
    }

    return 0;
}