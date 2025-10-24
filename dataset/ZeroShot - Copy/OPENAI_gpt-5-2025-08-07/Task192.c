#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

bool xorGame(const int* nums, size_t n) {
    if (nums == NULL) return false;
    int x = 0;
    for (size_t i = 0; i < n; ++i) {
        x ^= nums[i];
    }
    if (x == 0) return true;
    return (n % 2 == 0);
}

void runTest(const int* nums, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", nums[i]);
    }
    bool r = xorGame(nums, n);
    printf("] -> %s\n", r ? "true" : "false");
}

int main(void) {
    int a1[] = {1, 1, 2};
    int a2[] = {0, 1};
    int a3[] = {1, 2, 3};
    int a4[] = {5};
    int a5[] = {1, 2};

    runTest(a1, sizeof(a1) / sizeof(a1[0]));
    runTest(a2, sizeof(a2) / sizeof(a2[0]));
    runTest(a3, sizeof(a3) / sizeof(a3[0]));
    runTest(a4, sizeof(a4) / sizeof(a4[0]));
    runTest(a5, sizeof(a5) / sizeof(a5[0]));

    return 0;
}