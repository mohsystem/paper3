#include <stdio.h>
#include <stdlib.h>

int value_at_index(const int* arr, size_t len, long long index, int* out_success) {
    if (out_success == NULL) {
        return 0;
    }
    *out_success = 0;
    if (arr == NULL) {
        return 0;
    }
    if (index < 0) {
        return 0;
    }
    size_t uindex = (size_t)index;
    if (uindex >= len) {
        return 0;
    }
    *out_success = 1;
    return arr[uindex];
}

int main(void) {
    int data[] = {10, 20, 30, 40, 50};
    size_t len = sizeof(data) / sizeof(data[0]);
    long long testIndices[5] = {0, 2, 4, -1, 5};

    printf("Array: [");
    for (size_t i = 0; i < len; ++i) {
        printf("%d%s", data[i], (i + 1 < len) ? ", " : "");
    }
    printf("]\n");

    for (int i = 0; i < 5; ++i) {
        long long idx = testIndices[i];
        int ok = 0;
        int value = value_at_index(data, len, idx, &ok);
        if (ok) {
            printf("Test %d: index=%lld -> value=%d\n", i + 1, idx, value);
        } else {
            printf("Test %d: index=%lld -> error\n", i + 1, idx);
        }
    }

    return 0;
}