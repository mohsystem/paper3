#include <stdio.h>

unsigned long long factorial(unsigned int n) {
    unsigned long long result = 1ULL;
    for (unsigned int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    unsigned int tests[5] = {0, 1, 5, 10, 20};
    for (int i = 0; i < 5; ++i) {
        unsigned int t = tests[i];
        printf("%u! = %llu\n", t, factorial(t));
    }
    return 0;
}