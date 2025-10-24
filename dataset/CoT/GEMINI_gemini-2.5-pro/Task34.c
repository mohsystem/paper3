#include <stdio.h>
#include <stdbool.h> // For bool type
#include <math.h>    // For pow function

/**
 * @brief Checks if a number is a Narcissistic Number (Armstrong Number).
 * 
 * @param value The positive integer to check.
 * @return true if the number is narcissistic, false otherwise.
 */
bool narcissistic(int value) {
    // The prompt guarantees positive non-zero integers.
    if (value <= 0) {
        return false;
    }

    // Count number of digits arithmetically
    int numDigits = 0;
    int temp = value;
    do {
        temp /= 10;
        numDigits++;
    } while (temp > 0);
    
    // Calculate the sum of powers
    long long sum = 0; // Use long long to prevent potential overflow
    temp = value;
    while (temp > 0) {
        int digit = temp % 10;
        // Using round to handle potential floating-point inaccuracies from pow()
        sum += (long long)round(pow(digit, numDigits));
        temp /= 10;
    }

    return sum == value;
}

int main() {
    int test_cases[] = {7, 153, 371, 1634, 1652};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("Is %d narcissistic? %s\n", 
               test_cases[i], 
               narcissistic(test_cases[i]) ? "true" : "false");
    }

    return 0;
}