#include <iostream>

/**
 * @brief Finds the number of cubes 'n' such that the sum of the first 'n' cubes equals 'm'.
 *
 * Given a total volume 'm', this function determines the integer 'n' where
 * 1^3 + 2^3 + ... + n^3 = m.
 * If such an 'n' exists, it is returned. Otherwise, -1 is returned.
 *
 * @param m The total volume of the building, a long long integer.
 * @return The number of cubes 'n', or -1 if no such integer 'n' exists.
 */
long long findNb(long long m) {
    long long n = 0;
    while (m > 0) {
        n++;
        long long cube = n * n * n;
        m -= cube;
    }
    
    if (m == 0) {
        return n;
    } else {
        return -1;
    }
}

int main() {
    // Test cases
    // Expected: 45
    std::cout << findNb(1071225) << std::endl;
    // Expected: -1
    std::cout << findNb(91716553919377) << std::endl;
    // Expected: 2022
    std::cout << findNb(4183059834009) << std::endl;
    // Expected: 1
    std::cout << findNb(1) << std::endl;
    // Expected: 3
    std::cout << findNb(36) << std::endl;

    return 0;
}