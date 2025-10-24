
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

int countDigits(int number) {
    int count = 0;
    int temp = number;
    while (temp > 0) {
        count++;
        temp /= 10;
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
    printf("Test 1: isNarcissistic(153) = %s\\n", isNarcissistic(153) ? "true" : "false");
    printf("Test 2: isNarcissistic(1652) = %s\\n", isNarcissistic(1652) ? "true" : "false");
    printf("Test 3: isNarcissistic(9) = %s\\n", isNarcissistic(9) ? "true" : "false");
    printf("Test 4: isNarcissistic(9474) = %s\\n", isNarcissistic(9474) ? "true" : "false");
    printf("Test 5: isNarcissistic(1634) = %s\\n", isNarcissistic(1634) ? "true" : "false");
    
    return 0;
}
