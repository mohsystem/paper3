#include <stdio.h>

int persistence(long long num) {
    int count = 0;
    while (num >= 10) {
        count++;
        long long product = 1;
        long long tempNum = num;
        while (tempNum > 0) {
            product *= tempNum % 10;
            tempNum /= 10;
        }
        num = product;
    }
    return count;
}

int main() {
    // Test cases
    printf("Input: 39, Output: %d\n", persistence(39));     // Expected: 3
    printf("Input: 999, Output: %d\n", persistence(999));   // Expected: 4
    printf("Input: 4, Output: %d\n", persistence(4));       // Expected: 0
    printf("Input: 25, Output: %d\n", persistence(25));     // Expected: 2
    printf("Input: 679, Output: %d\n", persistence(679));   // Expected: 5
    return 0;
}