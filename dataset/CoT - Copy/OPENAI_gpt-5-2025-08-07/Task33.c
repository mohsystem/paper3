/* Step 1-5 (Chain-of-Through) summarized:
 * 1) Problem: Compute total checkout time with n tills and a single queue.
 * 2) Security: Validate parameters; avoid buffer overflows and leaks.
 * 3) Implementation: Track per-till loads; assign to minimum load till.
 * 4) Review: Free allocated memory; handle empty inputs; check n>0.
 * 5) Final: Returns integer; main has 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>

int queueTime(const int* customers, size_t len, int n) {
    if (customers == NULL || len == 0) return 0;
    if (n <= 0) {
        /* In real code, we might handle this differently; here return 0 safely. */
        return 0;
    }
    int* tills = (int*)calloc((size_t)n, sizeof(int));
    if (!tills) {
        /* Allocation failed; fail safely */
        return 0;
    }

    for (size_t i = 0; i < len; ++i) {
        int t = customers[i];
        /* find index of minimum load till */
        int min_idx = 0;
        int min_val = tills[0];
        for (int j = 1; j < n; ++j) {
            if (tills[j] < min_val) {
                min_val = tills[j];
                min_idx = j;
            }
        }
        /* assign customer to that till */
        tills[min_idx] += t;
    }

    int total = 0;
    for (int j = 0; j < n; ++j) {
        if (tills[j] > total) total = tills[j];
    }

    free(tills);
    return total;
}

int main(void) {
    int a1[] = {5, 3, 4};
    int a2[] = {10, 2, 3, 3};
    int a3[] = {2, 3, 10};
    int a4[] = {};
    int a5[] = {1, 2, 3, 4, 5};

    struct {
        const int* arr;
        size_t len;
        int n;
        int expected;
    } tests[5] = {
        {a1, sizeof(a1)/sizeof(a1[0]), 1, 12},
        {a2, sizeof(a2)/sizeof(a2[0]), 2, 10},
        {a3, sizeof(a3)/sizeof(a3[0]), 2, 12},
        {a4, 0, 1, 0},
        {a5, sizeof(a5)/sizeof(a5[0]), 100, 5}
    };

    for (int i = 0; i < 5; ++i) {
        int res = queueTime(tests[i].arr, tests[i].len, tests[i].n);
        printf("Test %d: %d (expected %d)\n", i + 1, res, tests[i].expected);
    }
    return 0;
}