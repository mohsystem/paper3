/* Chain-of-Through Process:
 * 1. Problem understanding: Decide if Alice wins under XOR game rules.
 * 2. Security requirements: Validate lengths; avoid buffer overruns; pure compute.
 * 3. Secure coding generation: No dynamic allocation; constant-time per element.
 * 4. Code review: XOR reduce and parity check.
 * 5. Secure code output: Final function and 5 tests.
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int xorGame(const uint32_t* nums, size_t n) {
    if (nums == NULL) {
        // Defensive: treat as empty; XOR=0 -> starting player wins.
        return 1;
    }
    if (n == 0) {
        // XOR of no elements is 0; starting player wins.
        return 1;
    }
    uint32_t x = 0u;
    for (size_t i = 0; i < n; ++i) {
        x ^= (nums[i] & 0xFFFFu);
    }
    if (x == 0u) return 1;
    return (n % 2u == 0u) ? 1 : 0;
}

static void runTest(const uint32_t* nums, size_t n) {
    printf("nums=[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%u", nums[i]);
    }
    int res = xorGame(nums, n);
    printf("] -> %s\n", res ? "true" : "false");
}

int main(void) {
    uint32_t a1[] = {1, 1, 2};    /* false */
    uint32_t a2[] = {0, 1};       /* true */
    uint32_t a3[] = {1, 2, 3};    /* true */
    uint32_t a4[] = {1};          /* false */
    uint32_t a5[] = {1, 1};       /* true */

    runTest(a1, sizeof(a1)/sizeof(a1[0]));
    runTest(a2, sizeof(a2)/sizeof(a2[0]));
    runTest(a3, sizeof(a3)/sizeof(a3[0]));
    runTest(a4, sizeof(a4)/sizeof(a4[0]));
    runTest(a5, sizeof(a5)/sizeof(a5[0]));
    return 0;
}