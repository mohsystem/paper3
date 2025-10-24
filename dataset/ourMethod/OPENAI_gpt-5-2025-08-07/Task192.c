#include <stdio.h>
#include <stddef.h>

static int validate_input(const int* nums, size_t len) {
    if (nums == NULL) return 0;
    if (len < 1 || len > 1000) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (nums[i] < 0 || nums[i] >= (1 << 16)) return 0;
    }
    return 1;
}

// Returns 1 if Alice wins, 0 otherwise. Invalid input returns 0 (fail closed).
int alice_wins(const int* nums, size_t len) {
    if (!validate_input(nums, len)) {
        return 0;
    }
    int xr = 0;
    for (size_t i = 0; i < len; ++i) {
        xr ^= nums[i];
    }
    if (xr == 0) return 1;
    return (len % 2 == 0) ? 1 : 0;
}

static void run_test(const int* nums, size_t len) {
    int res = alice_wins(nums, len);
    printf("Input: [");
    for (size_t i = 0; i < len; ++i) {
        printf("%d%s", nums[i], (i + 1 < len) ? "," : "");
    }
    printf("] -> %s\n", res ? "true" : "false");
}

int main(void) {
    // 5 test cases
    int a1[] = {1, 1, 2};     run_test(a1, sizeof(a1)/sizeof(a1[0])); // false
    int a2[] = {0, 1};        run_test(a2, sizeof(a2)/sizeof(a2[0])); // true
    int a3[] = {1, 2, 3};     run_test(a3, sizeof(a3)/sizeof(a3[0])); // true
    int a4[] = {1};           run_test(a4, sizeof(a4)/sizeof(a4[0])); // false
    int a5[] = {0};           run_test(a5, sizeof(a5)/sizeof(a5[0])); // true
    return 0;
}