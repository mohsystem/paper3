#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

bool alice_wins(const int *nums, size_t n) {
    // Validate inputs per constraints
    if (nums == NULL || n < 1 || n > 1000) {
        return false; // fail closed on invalid input
    }
    int xr = 0;
    for (size_t i = 0; i < n; ++i) {
        if (nums[i] < 0 || nums[i] >= (1 << 16)) {
            return false; // fail closed on invalid value
        }
        xr ^= nums[i];
    }
    // If xor is 0 at start, Alice wins. Else Alice wins iff length is even.
    if (xr == 0) return true;
    return (n % 2 == 0);
}

static void run_test(const int *nums, size_t n) {
    bool res = alice_wins(nums, n);
    printf("nums=[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", nums[i]);
    }
    printf("] -> %s\n", res ? "true" : "false");
}

int main(void) {
    // 5 test cases
    int a1[] = {1, 1, 2};
    int a2[] = {0, 1};
    int a3[] = {1, 2, 3};
    int a4[] = {5};
    int a5[] = {1, 1, 1};

    run_test(a1, sizeof(a1) / sizeof(a1[0])); // false
    run_test(a2, sizeof(a2) / sizeof(a2[0])); // true
    run_test(a3, sizeof(a3) / sizeof(a3[0])); // true
    run_test(a4, sizeof(a4) / sizeof(a4[0])); // false
    run_test(a5, sizeof(a5) / sizeof(a5[0])); // false

    return 0;
}