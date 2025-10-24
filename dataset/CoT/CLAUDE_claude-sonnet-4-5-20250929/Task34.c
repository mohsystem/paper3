
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

bool isNarcissistic(int number) {
    // Input validation - ensure positive integer
    if (number < 0) {
        return false;
    }
    
    // Count number of digits
    int temp = number;
    int numDigits = 0;
    while (temp > 0) {
        numDigits++;
        temp /= 10;
    }
    
    // Calculate sum of digits raised to power of digit count
    temp = number;
    int sum = 0;
    while (temp > 0) {
        int digit = temp % 10;
        sum += (int)pow(digit, numDigits);
        temp /= 10;
    }
    
    return sum == number;
}

int main() {
    // Test cases
    printf("Test 1 - 153: %s\\n", isNarcissistic(153) ? "true" : "false");
    printf("Test 2 - 1652: %s\\n", isNarcissistic(1652) ? "true" : "false");
    printf("Test 3 - 9: %s\\n", isNarcissistic(9) ? "true" : "false");
    printf("Test 4 - 9474: %s\\n", isNarcissistic(9474) ? "true" : "false");
    printf("Test 5 - 370: %s\\n", isNarcissistic(370) ? "true" : "false");
    
    return 0;
}
