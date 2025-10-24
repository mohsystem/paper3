#include <iostream>
#include <vector>
#include <stdexcept>
#include <climits> // For LLONG_MAX

/**
 * Converts a vector of binary digits (0s and 1s) to its integer equivalent.
 *
 * @param binary A constant reference to a vector of integers.
 * @return The long long representation of the binary value.
 * @throws std::invalid_argument if the vector contains elements other than 0 or 1.
 * @throws std::overflow_error if the resulting number exceeds the capacity of long long.
 */
long long binaryArrayToInteger(const std::vector<int>& binary) {
    long long result = 0;
    for (int bit : binary) {
        if (bit != 0 && bit != 1) {
            throw std::invalid_argument("Array must contain only 0s and 1s.");
        }
        // Check for potential overflow before left shift and addition
        if (result > (LLONG_MAX - bit) / 2) {
            throw std::overflow_error("Binary number is too large to fit in a long long.");
        }
        result = (result << 1) | bit;
    }
    return result;
}

void run_test_case(const std::vector<int>& test_case) {
    std::cout << "Testing: [";
    for(size_t i = 0; i < test_case.size(); ++i) {
        std::cout << test_case[i] << (i == test_case.size() - 1 ? "" : ", ");
    }
    std::cout << "] ==> ";
    try {
        long long result = binaryArrayToInteger(test_case);
        std::cout << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // 5 Test cases
    run_test_case({0, 0, 0, 1}); // ==> 1
    run_test_case({0, 0, 1, 0}); // ==> 2
    run_test_case({0, 1, 0, 1}); // ==> 5
    run_test_case({1, 0, 0, 1}); // ==> 9
    run_test_case({1, 1, 1, 1}); // ==> 15
    
    return 0;
}