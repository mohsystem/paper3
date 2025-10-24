#include <iostream>
#include <vector>
#include <climits>
#include <cstddef>

// Converts an array of binary digits to its integer equivalent.
//
// @param arr The input vector of integers, expected to be 0s and 1s.
// @return The integer value of the binary representation.
//         On error (invalid digit or overflow), it prints a message to stderr
//         and returns ULLONG_MAX. Note that ULLONG_MAX is a valid output
//         for an input of 64 '1's, making the error case ambiguous.
unsigned long long binaryArrayToInteger(const std::vector<int>& arr) {
    unsigned long long result = 0;
    for (int bit : arr) {
        // Input validation: ensure each element is a valid binary digit.
        if (bit != 0 && bit != 1) {
            std::cerr << "Error: Input array contains non-binary value." << std::endl;
            return ULLONG_MAX;
        }
        // Security check: prevent integer overflow before multiplication.
        if (result > (ULLONG_MAX - bit) / 2) {
            std::cerr << "Error: Overflow would occur during conversion." << std::endl;
            return ULLONG_MAX;
        }
        result = (result << 1) | bit;
    }
    return result;
}

// Helper function to run and print a test case.
void run_test(const std::vector<int>& arr) {
    std::cout << "Testing: [";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i == arr.size() - 1 ? "" : ", ");
    }
    std::cout << "] ==> " << binaryArrayToInteger(arr) << std::endl;
}

int main() {
    // 5 Test cases
    run_test({0, 0, 0, 1});
    run_test({0, 0, 1, 0});
    run_test({0, 1, 0, 1});
    run_test({1, 0, 0, 1});
    run_test({0, 1, 1, 0});
    return 0;
}