#include <stdio.h>
#include <stddef.h>

typedef struct {
    int ok;     /* 1 on success, 0 on error */
    int value;  /* valid only if ok == 1 */
} GetResult;

GetResult get_value_at_index(const int* arr, size_t len, long long index) {
    GetResult res;
    res.ok = 0;
    res.value = 0;

    if (arr == NULL) {
        return res;
    }
    if (index < 0) {
        return res;
    }
    if ((unsigned long long)index >= (unsigned long long)len) {
        return res;
    }
    res.ok = 1;
    res.value = arr[(size_t)index];
    return res;
}

int main(void) {
    int arr[] = {10, 20, 30, 40, 50};
    size_t len = sizeof(arr) / sizeof(arr[0]);
    long long tests[5] = {0, 2, 4, 5, -1};

    printf("Array: [");
    for (size_t i = 0; i < len; ++i) {
        printf("%d%s", arr[i], (i + 1 < len) ? ", " : "");
    }
    printf("]\n");

    for (size_t i = 0; i < 5; ++i) {
        long long idx = tests[i];
        GetResult r = get_value_at_index(arr, len, idx);
        if (r.ok) {
            printf("Index %lld -> %d\n", idx, r.value);
        } else {
            printf("Index %lld -> Error: Index out of bounds or invalid array\n", idx);
        }
    }

    return 0;
}