#include <iostream>
#include <vector>
#include <numeric>

/**
 * Converts a vector of binary digits (0s and 1s) to its integer equivalent.
 * @param arr A constant reference to a vector of integers.
 * @return The decimal integer value of the binary representation.
 */
int binaryArrayToNumber(const std::vector<int>& arr) {
    int number = 0;
    for (int bit : arr) {
        // Left shift the current number by 1 and add the new bit.
        // This is equivalent to number = number * 2 + bit;
        number = (number << 1) | bit;
    }
    return number;
}

int main() {
    // Test cases
    std::cout << binaryArrayToNumber({0, 0, 0, 1}) << std::endl; // Expected: 1
    std::cout << binaryArrayToNumber({0, 1, 0, 1}) << std::endl; // Expected: 5
    std::cout << binaryArrayToNumber({1, 0, 0, 1}) << std::endl; // Expected: 9
    std::cout << binaryArrayToNumber({1, 1, 1, 1}) << std::endl; // Expected: 15
    std::cout << binaryArrayToNumber({1, 0, 1, 1}) << std::endl; // Expected: 11
    return 0;
}