
#include <stdio.h>

int getSum(int a, int b) {
    if (a == b) {
        return a;
    }
    
    int minVal = (a < b) ? a : b;
    int maxVal = (a > b) ? a : b;
    
    // Using arithmetic progression formula: sum = n * (first + last) / 2
    int n = maxVal - minVal + 1;
    return n * (minVal + maxVal) / 2;
}

int main() {
    // Test case 1
    printf("Test 1 (1, 0): %d (Expected: 1)\\n", getSum(1, 0));
    
    // Test case 2
    printf("Test 2 (1, 2): %d (Expected: 3)\\n", getSum(1, 2));
    
    // Test case 3
    printf("Test 3 (0, 1): %d (Expected: 1)\\n", getSum(0, 1));
    
    // Test case 4
    printf("Test 4 (1, 1): %d (Expected: 1)\\n", getSum(1, 1));
    
    // Test case 5
    printf("Test 5 (-1, 2): %d (Expected: 2)\\n", getSum(-1, 2));
    
    return 0;
}
