#include <stdio.h>
#include <limits.h>
#include <stddef.h>

unsigned long long binaryArrayToInt(const int* arr, size_t n, int* err) {
    if (err) *err = 0;
    if (arr == NULL && n != 0) {
        if (err) *err = 1;
        return 0ULL;
    }
    unsigned long long acc = 0ULL;
    for (size_t i = 0; i < n; ++i) {
        int bit = arr[i];
        if (!(bit == 0 || bit == 1)) {
            if (err) *err = 2; // invalid bit
            return 0ULL;
        }
        if (acc > (ULLONG_MAX - (unsigned long long)bit) / 2ULL) {
            if (err) *err = 3; // overflow
            return 0ULL;
        }
        acc = (acc << 1) + (unsigned long long)bit;
    }
    return acc;
}

int main(void) {
    int err = 0;

    int t1[] = {0, 0, 0, 1}; // 1
    int t2[] = {0, 0, 1, 0}; // 2
    int t3[] = {0, 1, 0, 1}; // 5
    int t4[] = {1, 0, 0, 1}; // 9
    int t5[] = {1, 1, 1, 1}; // 15

    const int* tests[] = {t1, t2, t3, t4, t5};
    size_t sizes[] = {4, 4, 4, 4, 4};

    for (size_t i = 0; i < 5; ++i) {
        err = 0;
        unsigned long long result = binaryArrayToInt(tests[i], sizes[i], &err);
        printf("Testing: [");
        for (size_t j = 0; j < sizes[i]; ++j) {
            printf("%d%s", tests[i][j], (j + 1 < sizes[i]) ? ", " : "");
        }
        if (err == 0) {
            printf("] ==> %llu\n", result);
        } else {
            printf("] ==> Error (code %d)\n", err);
        }
    }
    return 0;
}