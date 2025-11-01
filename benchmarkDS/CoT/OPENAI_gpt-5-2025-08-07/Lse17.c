/* Chain-of-Through process:
1) Problem understanding: Safely get the value at a given index in an integer array.
2) Security requirements: Prevent out-of-bounds access and null dereference.
3) Secure coding generation: Validate inputs and bounds before access.
4) Code review: Checked all branches for safe memory access and proper signaling.
5) Secure code output: Returns a struct with success flag and value.
*/
#include <stdio.h>
#include <stddef.h>

typedef struct {
    int success; /* 1 if valid, 0 if invalid */
    int value;   /* valid only if success == 1 */
} IndexResult;

IndexResult get_value_at_index(const int* arr, size_t length, long index) {
    IndexResult res;
    res.success = 0;
    res.value = 0;

    if (arr == NULL) {
        return res;
    }
    if (index < 0) {
        return res;
    }
    /* Ensure index fits within size_t and bounds */
    size_t uindex = (size_t)index;
    if (uindex >= length) {
        return res;
    }

    res.success = 1;
    res.value = arr[uindex];
    return res;
}

int main(void) {
    int data[] = {10, 20, 30, 40, 50};
    size_t len = sizeof(data) / sizeof(data[0]);

    long testIndices[] = {0, 2, 4, -1, 10};
    size_t tests = sizeof(testIndices) / sizeof(testIndices[0]);

    for (size_t i = 0; i < tests; ++i) {
        long idx = testIndices[i];
        IndexResult r = get_value_at_index(data, len, idx);
        if (r.success) {
            printf("Index %ld -> %d\n", idx, r.value);
        } else {
            printf("Index %ld -> invalid index\n", idx);
        }
    }
    return 0;
}