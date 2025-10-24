#include <stdio.h>
#include <stdbool.h>
#include <math.h>

/**
 * Checks if a given number is a Narcissistic Number (Armstrong Number).
 *
 * @param number The positive integer to check.
 * @return true if the number is narcissistic, false otherwise.
 */
bool isNarcissistic(int number) {
    if (number <= 0) {
        return false;
    }

    int originalNumber = number;
    
    // Calculate number of digits
    int digits = 0;
    int temp_for_digits = number;
    do {
        temp_for_digits /= 10;
        digits++;
    } while (temp_for_digits > 0);
    
    long long sum = 0;
    int temp = originalNumber;

    while (temp > 0) {
        int digit = temp % 10;
        
        // Calculate power for the digit without using math.h to avoid floating point issues
        long long p = 1;
        for (int i = 0; i < digits; i++) {
            p *= digit;
        }
        sum += p;
        
        temp /= 10;
    }

    return sum == originalNumber;
}

int main() {
    int testCases[] = {153, 1652, 1, 371, 9474};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("%s\n", isNarcissistic(testCases[i]) ? "true" : "false");
    }

    return 0;
}