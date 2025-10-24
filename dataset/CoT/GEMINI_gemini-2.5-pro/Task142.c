#include <stdio.h>
#include <stdlib.h> // For abs()

/**
 * @brief Finds the greatest common divisor (GCD) of two integers using the Euclidean algorithm.
 * 
 * @param a The first integer.
 * @param b The second integer.
 * @return The greatest common divisor of a and b.
 */
int findGCD(int a, int b) {
    // The Euclidean algorithm
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    // The GCD is conventionally a non-negative number.
    return abs(a);
}

int main() {
    // 5 Test Cases
    int testCases[][2] = {
        {54, 24},     // Basic case with two positive numbers
        {100, 0},     // Case with zero
        {-48, 18},    // Case with a negative number
        {17, 23},     // Case with two prime numbers
        {0, 0}        // Case where both numbers are zero
    };
    
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        int num1 = testCases[i][0];
        int num2 = testCases[i][1];
        int result = findGCD(num1, num2);
        printf("The GCD of %d and %d is: %d\n", num1, num2, result);
    }

    return 0;
}