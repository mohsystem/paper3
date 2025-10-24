#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

long long allocate_and_process(const int* data, size_t n) {
    if (data == NULL && n != 0) {
        // Invalid input pointer
        return 0LL;
    }

    // Secure overflow check for allocation size
    if (n > SIZE_MAX / sizeof(int)) {
        // Allocation would overflow
        return 0LL;
    }

    int* arr = NULL;
    if (n > 0) {
        arr = (int*)malloc(n * sizeof(int));
        if (arr == NULL) {
            // Allocation failed
            return 0LL;
        }
    }

    // Copy data into allocated buffer
    for (size_t i = 0; i < n; ++i) {
        arr[i] = data[i];
    }

    // Process: compute sum reading from allocated memory
    long long sum = 0;
    for (size_t i = 0; i < n; ++i) {
        sum += (long long)arr[i];
    }

    // Deallocate
    free(arr);
    return sum;
}

int main(void) {
    // 5 test cases
    int t1[] = {}; // empty
    int t2[] = {1, 2, 3, 4, 5};
    int t3[] = {INT_MAX, 1, -2};

    int t4[1000];
    for (int i = 0, v = -500; i < 1000; ++i, ++v) t4[i] = v;

    int t5[] = {1000000000, 1000000000, 1000000000};

    struct {
        const int* data;
        size_t n;
    } tests[] = {
        {t1, 0},
        {t2, sizeof(t2)/sizeof(t2[0])},
        {t3, sizeof(t3)/sizeof(t3[0])},
        {t4, sizeof(t4)/sizeof(t4[0])},
        {t5, sizeof(t5)/sizeof(t5[0])}
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        long long result = allocate_and_process(tests[i].data, tests[i].n);
        printf("Test %zu sum = %lld\n", i + 1, result);
    }

    return 0;
}