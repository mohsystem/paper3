
#include <stdio.h>

long long factorial(int n) {
    if (n < 0) {
        printf("Error: Factorial is not defined for negative numbers\\n");
        return -1;
    }
    if (n == 0 || n == 1) {
        return 1;
    }
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int main() {
    // Test case 1
    printf("Factorial of 0: %lld\\n", factorial(0));
    
    // Test case 2
    printf("Factorial of 1: %lld\\n", factorial(1));
    
    // Test case 3
    printf("Factorial of 5: %lld\\n", factorial(5));
    
    // Test case 4
    printf("Factorial of 10: %lld\\n", factorial(10));
    
    // Test case 5
    printf("Factorial of 15: %lld\\n", factorial(15));
    
    return 0;
}
