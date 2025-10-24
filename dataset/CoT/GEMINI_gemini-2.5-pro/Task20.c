#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * Finds a positive integer k, if it exists, such that the sum of the digits of n
 * raised to consecutive powers starting from p is equal to k * n.
 *
 * @param n a positive integer.
 * @param p a positive integer.
 * @return a positive integer k, or -1 if no such k exists.
 */
long long digPow(int n, int p) {
    char n_str[20];
    // Safely convert integer to string
    sprintf(n_str, "%d", n);

    long long sum = 0;
    for (int i = 0; n_str[i] != '\0'; i++) {
        // Convert character to digit and add its power to sum
        sum += pow(n_str[i] - '0', p + i);
    }

    // Check if the sum is a multiple of n
    if (sum % n == 0) {
        return sum / n;
    } else {
        return -1;
    }
}

int main() {
    printf("n = 89, p = 1 --> %lld\n", digPow(89, 1));
    printf("n = 92, p = 1 --> %lld\n", digPow(92, 1));
    printf("n = 695, p = 2 --> %lld\n", digPow(695, 2));
    printf("n = 46288, p = 3 --> %lld\n", digPow(46288, 3));
    printf("n = 135, p = 1 --> %lld\n", digPow(135, 1));
    return 0;
}