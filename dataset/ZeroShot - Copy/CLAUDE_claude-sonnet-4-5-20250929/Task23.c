
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
    printf("Test 1: findNb(1071225) = %lld\\n", findNb(1071225)); // Expected: 45
    printf("Test 2: findNb(91716553919377) = %lld\\n", findNb(91716553919377LL)); // Expected: -1
    printf("Test 3: findNb(4183059834009) = %lld\\n", findNb(4183059834009LL)); // Expected: 2022
    printf("Test 4: findNb(24723578342962) = %lld\\n", findNb(24723578342962LL)); // Expected: -1
    printf("Test 5: findNb(135) = %lld\\n", findNb(135)); // Expected: 5
    
    return 0;
}
