#include <stdio.h>
#include <stdbool.h>

// Helper function to calculate integer power securely, avoiding floating-point math.
long long power(int base, int exp) {
    long long res = 1;
    for (int i = 0; i < exp; ++i) {
        res *= base;
    }
    return res;
}

/**
 * @brief Checks if a number is a Narcissistic Number.
 * 
 * A Narcissistic Number is a positive number which is the sum of its own digits,
 * each raised to the power of the number of digits.
 * @param value The positive integer to check.
 * @return true if the number is narcissistic, false otherwise.
 */
bool is_narcissistic(int value) {
    if (value <= 0) {
        return false;
    }
    
    // Count the number of digits
    int temp = value;
    int numDigits = 0;
    while(temp > 0) {
        temp /= 10;
        numDigits++;
    }

    // Calculate the sum of powers of digits
    long long sum = 0;
    temp = value;
    while (temp > 0) {
        int digit = temp % 10;
        sum += power(digit, numDigits);
        // Early exit if sum exceeds value
        if (sum > value) {
            return false;
        }
        temp /= 10;
    }

    return sum == value;
}

int main() {
    int testCases[] = {153, 1652, 7, 371, 9474};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTestCases; i++) {
        printf("is_narcissistic(%d) -> %s\n", 
               testCases[i], 
               is_narcissistic(testCases[i]) ? "true" : "false");
    }
    return 0;
}