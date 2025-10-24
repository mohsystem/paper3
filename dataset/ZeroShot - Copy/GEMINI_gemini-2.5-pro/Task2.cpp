#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>

/**
 * @brief Formats a vector of 10 integers (0-9) into a phone number string.
 *
 * @param numbers The vector of 10 integers.
 * @return A formatted string like "(123) 456-7890".
 * @throw std::invalid_argument if the input is invalid.
 */
std::string createPhoneNumber(const std::vector<int>& numbers) {
    // Security: Validate input size.
    if (numbers.size() != 10) {
        throw std::invalid_argument("Input vector must contain exactly 10 integers.");
    }

    std::stringstream ss;
    ss << "(";
    for (size_t i = 0; i < numbers.size(); ++i) {
        int num = numbers[i];
        // Security: Ensure all numbers are single digits (0-9).
        if (num < 0 || num > 9) {
            throw std::invalid_argument("All numbers in the vector must be between 0 and 9.");
        }
        ss << num;
        if (i == 2) {
            ss << ") ";
        } else if (i == 5) {
            ss << "-";
        }
    }
    return ss.str();
}

int main() {
    // Test Case 1: Valid input
    std::vector<int> case1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
    std::cout << "Test Case 1: " << createPhoneNumber(case1) << std::endl;

    // Test Case 2: Valid input with zeros
    std::vector<int> case2 = {0, 0, 0, 1, 1, 1, 2, 2, 2, 2};
    std::cout << "Test Case 2: " << createPhoneNumber(case2) << std::endl;

    // Test Case 3: Invalid length (too short)
    try {
        std::vector<int> case3 = {1, 2, 3};
        std::cout << "Test Case 3: " << createPhoneNumber(case3) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test Case 3: Caught expected exception - " << e.what() << std::endl;
    }

    // Test Case 4: Invalid number (out of range)
    try {
        std::vector<int> case4 = {1, 2, 3, 4, 5, 6, 7, 99, 9, 0};
        std::cout << "Test Case 4: " << createPhoneNumber(case4) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test Case 4: Caught expected exception - " << e.what() << std::endl;
    }

    // Test Case 5: Empty vector
    try {
        std::vector<int> case5;
        std::cout << "Test Case 5: " << createPhoneNumber(case5) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test Case 5: Caught expected exception - " << e.what() << std::endl;
    }

    return 0;
}