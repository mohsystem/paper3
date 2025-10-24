#include <stdio.h>
#include <stdlib.h>

const char* odd_or_even(const long long* arr, size_t len) {
    if (arr == NULL || len == 0) return "even";
    int parity = 0;
    for (size_t i = 0; i < len; ++i) {
        parity ^= (llabs(arr[i]) & 1LL);
    }
    return parity == 0 ? "even" : "odd";
}

int main() {
    long long t0[] = {};
    long long t1[] = {0};
    long long t2[] = {0, 1, 4};
    long long t3[] = {0, -1, -5};
    long long t4[] = {1, 2, 3, 4, 5};

    printf("%s\n", odd_or_even(t0, 0));
    printf("%s\n", odd_or_even(t1, sizeof(t1)/sizeof(t1[0])));
    printf("%s\n", odd_or_even(t2, sizeof(t2)/sizeof(t2[0])));
    printf("%s\n", odd_or_even(t3, sizeof(t3)/sizeof(t3[0])));
    printf("%s\n", odd_or_even(t4, sizeof(t4)/sizeof(t4[0])));
    return 0;
}