
#include <stdio.h>

int persistence(int num) {
    int count = 0;
    
    while (num >= 10) {
        int product = 1;
        while (num > 0) {
            product *= num % 10;
            num /= 10;
        }
        num = product;
        count++;
    }
    
    return count;
}

int main() {
    // Test cases
    printf("Input: 39, Output: %d\\n", persistence(39));
    printf("Input: 999, Output: %d\\n", persistence(999));
    printf("Input: 4, Output: %d\\n", persistence(4));
    printf("Input: 25, Output: %d\\n", persistence(25));
    printf("Input: 77, Output: %d\\n", persistence(77));
    
    return 0;
}
