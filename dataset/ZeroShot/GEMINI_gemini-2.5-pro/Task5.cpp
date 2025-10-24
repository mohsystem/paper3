#include <iostream>
#include <stdexcept>

/**
 * Calculates the multiplicative persistence of a positive number.
 *
 * @param num A non-negative long long integer.
 * @return The multiplicative persistence of num.
 * @throws std::invalid_argument if the input number is negative.
 */
int persistence(long long num) {
    // Secure coding: Validate input to ensure it's non-negative.
    if (num < 0) {
        throw std::invalid_argument("Input must be a non-negative number.");
    }

    int count = 0;
    while (num >= 10) {
        count++;
        long long product = 1;
        long long temp = num;
        while (temp > 0) {
            product *= temp % 10;
            temp /= 10;
        }
        num = product;
    }
    return count;
}

int main() {
    // 5 Test Cases
    try {
        std::cout << "persistence(39) -> " << persistence(39) << std::endl;      // Expected: 3
        std::cout << "persistence(999) -> " << persistence(999) << std::endl;    // Expected: 4
        std::cout << "persistence(4) -> " << persistence(4) << std::endl;        // Expected: 0
        std::cout << "persistence(25) -> " << persistence(25) << std::endl;      // Expected: 2
        std::cout << "persistence(679) -> " << persistence(679) << std::endl;    // Expected: 5
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1; // Indicate error
    }
    return 0;
}