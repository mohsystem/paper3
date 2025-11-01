#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int ok;    /* 1 on success, 0 on error */
    int value; /* valid only if ok == 1 */
} IndexResult;

IndexResult get_value_at_index(int index, const int* arr, size_t len) {
    IndexResult res;
    res.ok = 0;
    res.value = 0;

    if (arr == NULL) {
        return res;
    }
    if (index < 0) {
        return res;
    }
    size_t uindex = (size_t)index;
    if (uindex >= len) {
        return res;
    }
    res.ok = 1;
    res.value = arr[uindex];
    return res;
}

int main(void) {
    int arr[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int tests[5] = {0, 5, 9, 3, 10}; /* 10 is invalid to demonstrate error handling */
    size_t len = sizeof(arr) / sizeof(arr[0]);

    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        IndexResult r = get_value_at_index(tests[i], arr, len);
        if (r.ok) {
            printf("index %d -> %d\n", tests[i], r.value);
        } else {
            printf("index %d -> error\n", tests[i]);
        }
    }
    return 0;
}