
#include <stdio.h>
#include <math.h>
#include <string.h>

long long digPow(int n, int p) {
    char numStr[20];
    sprintf(numStr, "%d", n);
    long long sum = 0;
    int len = strlen(numStr);
    
    for (int i = 0; i < len; i++) {
        int digit = numStr[i] - '0';
        sum += (long long)pow(digit, p + i);
    }
    
    if (sum % n == 0) {
        return sum / n;
    }
    return -1;
}

int main() {
    // Test case 1
    printf("Test 1: n=89, p=1 -> %lld\\n", digPow(89, 1)); // Expected: 1
    
    // Test case 2
    printf("Test 2: n=92, p=1 -> %lld\\n", digPow(92, 1)); // Expected: -1
    
    // Test case 3
    printf("Test 3: n=695, p=2 -> %lld\\n", digPow(695, 2)); // Expected: 2
    
    // Test case 4
    printf("Test 4: n=46288, p=3 -> %lld\\n", digPow(46288, 3)); // Expected: 51
    
    // Test case 5
    printf("Test 5: n=114, p=3 -> %lld\\n", digPow(114, 3)); // Expected: 9
    
    return 0;
}
