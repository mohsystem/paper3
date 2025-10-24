#include <stdio.h>

int persistence(unsigned long long num) {
    if (num < 10ULL) return 0;
    int count = 0;
    while (num >= 10ULL) {
        unsigned long long prod = 1;
        unsigned long long n = num;
        while (n > 0ULL) {
            prod *= (n % 10ULL);
            n /= 10ULL;
        }
        num = prod;
        count++;
    }
    return count;
}

int main() {
    unsigned long long tests[5] = {39, 999, 4, 25, 777};
    for (int i = 0; i < 5; i++) {
        printf("persistence(%llu) = %d\n", tests[i], persistence(tests[i]));
    }
    return 0;
}