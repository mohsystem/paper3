
#include <stdio.h>

long long findNb(long long m) {
    long long n = 0;
    long long sum = 0;
    
    while (sum < m) {
        n++;
        sum += n * n * n;
    }
    
    return sum == m ? n : -1;
}

int main() {
    // Test cases
    printf("Test 1: %lld (Expected: 45)\\n", findNb(1071225));
    printf("Test 2: %lld (Expected: -1)\\n", findNb(91716553919377LL));
    printf("Test 3: %lld (Expected: 2022)\\n", findNb(4183059834009LL));
    printf("Test 4: %lld (Expected: -1)\\n", findNb(24723578342962LL));
    printf("Test 5: %lld (Expected: 5)\\n", findNb(135));
    
    return 0;
}
