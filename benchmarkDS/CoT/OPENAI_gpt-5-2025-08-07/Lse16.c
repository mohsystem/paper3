/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Return value at given index from a predefined array.
 * 2) Security requirements: Prevent out-of-bounds and invalid memory access.
 * 3) Secure coding generation: Validate inputs; use clear return codes; avoid UB.
 * 4) Code review: Ensure proper bounds checks and null pointer checks.
 * 5) Secure code output: Final function using safe checks and predictable behavior.
 */

#include <stdio.h>

static const int DATA[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
#define DATA_LEN (sizeof(DATA) / sizeof(DATA[0]))

/* Returns 0 on success and writes the value to *out_value.
 * Returns -1 on error (invalid index or null out_value).
 */
int value_at_index(int index, int *out_value) {
    if (out_value == NULL) {
        return -1;
    }
    if (index < 0 || (unsigned int)index >= DATA_LEN) {
        return -1;
    }
    *out_value = DATA[(unsigned int)index];
    return 0;
}

int main(void) {
    int tests[5] = {0, 5, 9, 3, 7};
    for (int i = 0; i < 5; ++i) {
        int idx = tests[i];
        int val = 0;
        int rc = value_at_index(idx, &val);
        if (rc == 0) {
            printf("Index %d -> %d\n", idx, val);
        } else {
            printf("Error for index %d: out of bounds\n", idx);
        }
    }
    return 0;
}