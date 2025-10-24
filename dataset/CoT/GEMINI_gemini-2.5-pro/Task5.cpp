#include <iostream>

/**
 * @brief Calculates the multiplicative persistence of a positive number.
 * 
 * @param n A positive long long integer.
 * @return The number of times digits must be multiplied to reach a single digit.
 */
int persistence(long long n) {
    int count = 0;
    while (n >= 10) {
        count++;
        long long product = 1;
        long long temp = n;
        while (temp > 0) {
            product *= temp % 10;
            temp /= 10;
        }
        n = product;
    }
    return count;
}

int main() {
    // Test cases
    long long testCases[] = {39, 999, 4, 25, 679};
    int expectedResults[] = {3, 4, 0, 2, 5};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        long long input = testCases[i];
        int expected = expectedResults[i];
        int result = persistence(input);
        std::cout << "Input: " << input << ", Output: " << result << ", Expected: " << expected << std::endl;
    }
    
    return 0;
}