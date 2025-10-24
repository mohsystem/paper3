#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

bool xorGame(const int* nums, size_t n) {
    if (nums == NULL || n == 0) {
        return true;
    }
    int x = 0;
    for (size_t i = 0; i < n; i++) {
        x ^= nums[i];
    }
    if (x == 0) {
        return true;
    }
    return (n % 2u) == 0u;
}

int main(void) {
    int t1[] = {1, 1, 2};
    int t2[] = {0, 1};
    int t3[] = {1, 2, 3};
    int t4[] = {0};
    int t5[] = {1};

    const int* tests[] = {t1, t2, t3, t4, t5};
    size_t sizes[] = {sizeof(t1)/sizeof(t1[0]), sizeof(t2)/sizeof(t2[0]), sizeof(t3)/sizeof(t3[0]), sizeof(t4)/sizeof(t4[0]), sizeof(t5)/sizeof(t5[0])};

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        printf("%s\n", xorGame(tests[i], sizes[i]) ? "true" : "false");
    }
    return 0;
}