#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int findMissing(const int* arr, size_t len, int n) {
    if (n < 1 || (int)len != n - 1) {
        return -1;
    }
    if (len > 0 && arr == NULL) {
        return -1;
    }

    unsigned char* seen = (unsigned char*)calloc((size_t)n + 1, sizeof(unsigned char));
    if (seen == NULL) {
        return -1;
    }

    int x = 0;
    for (int i = 1; i <= n; ++i) {
        x ^= i;
    }

    for (size_t i = 0; i < len; ++i) {
        int v = arr[i];
        if (v < 1 || v > n) {
            free(seen);
            return -1;
        }
        if (seen[(size_t)v]) {
            free(seen);
            return -1;
        }
        seen[(size_t)v] = 1;
        x ^= v;
    }

    if (x < 1 || x > n || seen[(size_t)x]) {
        free(seen);
        return -1;
    }

    free(seen);
    return x;
}

int main(void) {
    int t1[] = {1, 2, 3, 5};                 // n=5 -> 4
    int* t2 = NULL;                           // n=1 -> 1 (empty array)
    int t3[] = {7, 6, 5, 4, 3, 2};            // n=7 -> 1
    int t4[] = {1, 2, 2};                     // n=4 -> -1 (duplicate)
    int t5[] = {0, 1, 2, 3, 4};               // n=6 -> -1 (out of range)

    printf("%d\n", findMissing(t1, sizeof(t1)/sizeof(t1[0]), 5));
    printf("%d\n", findMissing(t2, 0, 1));
    printf("%d\n", findMissing(t3, sizeof(t3)/sizeof(t3[0]), 7));
    printf("%d\n", findMissing(t4, sizeof(t4)/sizeof(t4[0]), 4));
    printf("%d\n", findMissing(t5, sizeof(t5)/sizeof(t5[0]), 6));

    return 0;
}