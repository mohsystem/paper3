#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

static inline bool is_even_ll(long long x) {
    return (x & 1LL) == 0;
}

// Returns 0 on success and writes outlier to *out_value.
// Returns non-zero on error (invalid inputs or ambiguous outlier).
int find_outlier(const long long* arr, size_t n, long long* out_value) {
    if (arr == NULL || out_value == NULL || n < 3) {
        return 1; // fail closed
    }

    size_t even_count = 0;
    size_t odd_count = 0;
    for (size_t i = 0; i < 3 && i < n; ++i) {
        if (is_even_ll(arr[i])) {
            ++even_count;
        } else {
            ++odd_count;
        }
    }

    bool majority_even = even_count >= 2;

    long long candidate = 0;
    size_t outliers_found = 0;
    for (size_t i = 0; i < n; ++i) {
        bool curr_even = is_even_ll(arr[i]);
        if (curr_even != majority_even) {
            candidate = arr[i];
            ++outliers_found;
            if (outliers_found > 1) {
                return 2; // multiple outliers found -> ambiguous
            }
        }
    }

    if (outliers_found == 1) {
        *out_value = candidate;
        return 0;
    }
    return 3; // none found
}

int main(void) {
    long long out = 0;

    long long t1[] = {2, 4, 0, 100, 4, 11, 2602, 36};          // outlier: 11
    if (find_outlier(t1, sizeof(t1)/sizeof(t1[0]), &out) == 0)
        printf("Test 1 outlier: %lld\n", out);
    else
        printf("Test 1 error\n");

    long long t2[] = {160, 3, 1719, 19, 11, 13, -21};          // outlier: 160
    if (find_outlier(t2, sizeof(t2)/sizeof(t2[0]), &out) == 0)
        printf("Test 2 outlier: %lld\n", out);
    else
        printf("Test 2 error\n");

    long long t3[] = {2, 6, 8, -10, 12, -14, 15, 16, 18, 20};  // outlier: 15
    if (find_outlier(t3, sizeof(t3)/sizeof(t3[0]), &out) == 0)
        printf("Test 3 outlier: %lld\n", out);
    else
        printf("Test 3 error\n");

    long long t4[] = {1, 3, 5, 7, 4, 9, 11};                    // outlier: 4
    if (find_outlier(t4, sizeof(t4)/sizeof(t4[0]), &out) == 0)
        printf("Test 4 outlier: %lld\n", out);
    else
        printf("Test 4 error\n");

    long long t5[] = {-1, -3, -5, -7, -9, -1001, -2002};       // outlier: -2002
    if (find_outlier(t5, sizeof(t5)/sizeof(t5[0]), &out) == 0)
        printf("Test 5 outlier: %lld\n", out);
    else
        printf("Test 5 error\n");

    return 0;
}