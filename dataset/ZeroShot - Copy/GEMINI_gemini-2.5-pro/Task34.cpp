#include <iostream>

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
bool isNarcissistic(int value) {
    if (value <= 0) {
        return false;
    }

    // Count the number of digits
    int temp = value;
    int numDigits = 0;
    while (temp > 0) {
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
    for (int testCase : testCases) {
        std::cout << "isNarcissistic(" << testCase << ") -> " 
                  << (isNarcissistic(testCase) ? "true" : "false") << std::endl;
    }
    return 0;
}