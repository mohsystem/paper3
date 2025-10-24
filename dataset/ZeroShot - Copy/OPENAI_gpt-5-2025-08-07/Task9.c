#include <stdio.h>
#include <stddef.h>

const char* odd_or_even(const int* arr, size_t len) {
    if (arr == NULL || len == 0) {
        return "even";
    }
    unsigned int parity = 0u;
    for (size_t i = 0; i < len; ++i) {
        parity ^= ((unsigned int)arr[i]) & 1u;
    }
    return parity ? "odd" : "even";
}

int main(void) {
    int a1[] = {0};
    int a2[] = {0, 1, 4};
    int a3[] = {0, -1, -5};
    /* Empty array represented with NULL and length 0 for testing */
    int a5[] = {2, 2, 2, 3};

    printf("[0] -> %s\n", odd_or_even(a1, sizeof(a1) / sizeof(a1[0])));
    printf("[0, 1, 4] -> %s\n", odd_or_even(a2, sizeof(a2) / sizeof(a2[0])));
    printf("[0, -1, -5] -> %s\n", odd_or_even(a3, sizeof(a3) / sizeof(a3[0])));
    printf("[] -> %s\n", odd_or_even(NULL, 0));
    printf("[2, 2, 2, 3] -> %s\n", odd_or_even(a5, sizeof(a5) / sizeof(a5[0])));

    return 0;
}