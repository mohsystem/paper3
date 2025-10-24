#include <stdio.h>
#include <string.h>
#include <math.h>

int digPow(int n, int p) {
    char s[20];
    sprintf(s, "%d", n);
    
    long long sum = 0;
    for (size_t i = 0; i < strlen(s); ++i) {
        int digit = s[i] - '0';
        sum += (long long)round(pow(digit, p + i));
    }

    if (sum % n == 0) {
        return (int)(sum / n);
    } else {
        return -1;
    }
}

int main() {
    printf("n=89, p=1 -> %d\n", digPow(89, 1));
    printf("n=92, p=1 -> %d\n", digPow(92, 1));
    printf("n=695, p=2 -> %d\n", digPow(695, 2));
    printf("n=46288, p=3 -> %d\n", digPow(46288, 3));
    printf("n=114, p=3 -> %d\n", digPow(114, 3));
    return 0;
}