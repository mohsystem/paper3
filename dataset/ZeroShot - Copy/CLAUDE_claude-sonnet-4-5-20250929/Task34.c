
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

int countDigits(int number) {
    int count = 0;
    while (number > 0) {
        count++;
        number /= 10;
    }
    return count;
}

bool isNarcissistic(int number) {
    int numDigits = countDigits(number);
    int sum = 0;
    int temp = number;
    
    while (temp > 0) {
        int digit = temp % 10;
        sum += (int)pow(digit, numDigits);
        temp /= 10;
    }
    
    return sum == number;
}

int main() {
    // Test cases
    printf("Test 1 - Input: 153, Output: %s\\n", isNarcissistic(153) ? "true" : "false");
    printf("Test 2 - Input: 1652, Output: %s\\n", isNarcissistic(1652) ? "true" : "false");
    printf("Test 3 - Input: 9474, Output: %s\\n", isNarcissistic(9474) ? "true" : "false");
    printf("Test 4 - Input: 9475, Output: %s\\n", isNarcissistic(9475) ? "true" : "false");
    printf("Test 5 - Input: 9, Output: %s\\n", isNarcissistic(9) ? "true" : "false");
    
    return 0;
}
