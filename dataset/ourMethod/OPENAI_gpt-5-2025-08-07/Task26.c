#include <stdio.h>
#include <stddef.h>

int find_odd(const int* arr, size_t len) {
    if (arr == NULL || len == 0) {
        return 0; /* Fail closed: return 0 if invalid input */
    }
    int x = 0;
    for (size_t i = 0; i < len; ++i) {
        x ^= arr[i];
    }
    return x;
}

int main(void) {
    int a1[] = {7};
    int a2[] = {0};
    int a3[] = {1, 1, 2};
    int a4[] = {0, 1, 0, 1, 0};
    int a5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};

    printf("Result 1: %d\n", find_odd(a1, sizeof(a1) / sizeof(a1[0])));
    printf("Result 2: %d\n", find_odd(a2, sizeof(a2) / sizeof(a2[0])));
    printf("Result 3: %d\n", find_odd(a3, sizeof(a3) / sizeof(a3[0])));
    printf("Result 4: %d\n", find_odd(a4, sizeof(a4) / sizeof(a4[0])));
    printf("Result 5: %d\n", find_odd(a5, sizeof(a5) / sizeof(a5[0])));

    return 0;
}