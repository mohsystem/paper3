
#include <stdio.h>

int persistence(int num) {
    if (num < 10) {
        return 0;
    }
    
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
    printf("Test 1: persistence(39) = %d (Expected: 3)\\n", persistence(39));
    printf("Test 2: persistence(999) = %d (Expected: 4)\\n", persistence(999));
    printf("Test 3: persistence(4) = %d (Expected: 0)\\n", persistence(4));
    printf("Test 4: persistence(25) = %d (Expected: 2)\\n", persistence(25));
    printf("Test 5: persistence(77) = %d (Expected: 4)\\n", persistence(77));
    
    return 0;
}
