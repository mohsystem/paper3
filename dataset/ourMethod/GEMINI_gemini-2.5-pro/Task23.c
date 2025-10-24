#include <stdio.h>

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
long long find_nb(long long m) {
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
    printf("%lld\n", find_nb(1071225));
    // Expected: -1
    printf("%lld\n", find_nb(91716553919377));
    // Expected: 2022
    printf("%lld\n", find_nb(4183059834009));
    // Expected: 1
    printf("%lld\n", find_nb(1));
    // Expected: 3
    printf("%lld\n", find_nb(36));
    
    return 0;
}