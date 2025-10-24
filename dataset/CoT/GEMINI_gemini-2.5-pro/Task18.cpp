#include <iostream>
#include <vector>

/**
 * @brief Given a vector of ones and zeroes, convert the equivalent binary value to an integer.
 * 
 * @param binary A constant reference to a vector of integers (0s and 1s).
 * @return The integer representation of the binary value.
 */
int binaryArrayToNumber(const std::vector<int>& binary) {
    int number = 0;
    for (int bit : binary) {
        // Left shift the current number by 1 and add the new bit using bitwise OR.
        // This is equivalent to number = number * 2 + bit;
        number = (number << 1) | bit;
    }
    return number;
}

int main() {
    // Test Case 1
    std::cout << binaryArrayToNumber({0, 0, 0, 1}) << std::endl; // Expected: 1
    // Test Case 2
    std::cout << binaryArrayToNumber({0, 0, 1, 0}) << std::endl; // Expected: 2
    // Test Case 3
    std::cout << binaryArrayToNumber({0, 1, 0, 1}) << std::endl; // Expected: 5
    // Test Case 4
    std::cout << binaryArrayToNumber({1, 0, 0, 1}) << std::endl; // Expected: 9
    // Test Case 5
    std::cout << binaryArrayToNumber({1, 1, 1, 1}) << std::endl; // Expected: 15

    return 0;
}