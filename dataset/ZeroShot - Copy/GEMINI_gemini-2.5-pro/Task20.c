#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * @brief Finds a positive integer k, if it exists, such that the sum of the digits of n
 * raised to consecutive powers starting from p is equal to k * n.
 * 
 * @param n A positive integer.
 * @param p A positive integer.
 * @return The integer k if found, otherwise -1.
 */
long long digPow(int n, int p) {
    char s[20];
    snprintf(s, sizeof(s), "%d", n);

    long long sum = 0;
    for (size_t i = 0; i < strlen(s); i++) {
        int digit = s[i] - '0';
        sum += (long long)round(pow(digit, p + i));
    }
    
    if (n > 0 && sum % n == 0) {
        return sum / n;
    } else {
        return -1;
    }
}

int main() {
    // Test cases
    printf("n=89, p=1 -> %lld\n", digPow(89, 1));
    printf("n=92, p=1 -> %lld\n", digPow(92, 1));
    printf("n=695, p=2 -> %lld\n", digPow(695, 2));
    printf("n=46288, p=3 -> %lld\n", digPow(46288, 3));
    printf("n=135, p=1 -> %lld\n", digPow(135, 1));
    return 0;
}