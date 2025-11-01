#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

// Safe addition for long long with overflow detection.
// Returns true if overflow occurred; result is only valid when false.
static bool add_ll_safe(long long a, long long b, long long* result) {
    if (result == NULL) return true;
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
        return true;
    }
    *result = a + b;
    return false;
}

long long getMonthlySales(const long long* sales, size_t len, size_t i) {
    if (sales == NULL) {
        fprintf(stderr, "Sales array cannot be NULL\n");
        exit(EXIT_FAILURE);
    }
    if (i >= len) {
        fprintf(stderr, "Index out of bounds for sales array\n");
        exit(EXIT_FAILURE);
    }
    return sales[i];
}

long long totalFirstQuarterSales(const long long* sales, size_t len) {
    if (sales == NULL || len < 3) {
        fprintf(stderr, "Sales array must have at least 3 elements\n");
        exit(EXIT_FAILURE);
    }
    long long sum = 0;
    for (size_t i = 0; i < 3; ++i) {
        long long val = getMonthlySales(sales, len, i);
        long long tmp = 0;
        if (add_ll_safe(sum, val, &tmp)) {
            fprintf(stderr, "Overflow during summation\n");
            exit(EXIT_FAILURE);
        }
        sum = tmp;
    }
    return sum;
}

int main(void) {
    long long test1[] = {100, 200, 300, 400, 500};                         // Expected: 600
    long long test2[] = {0, 0, 0};                                         // Expected: 0
    long long test3[] = {12345, 67890, 11111};                             // Expected: 91346
    long long test4[] = {LLONG_MAX, 0, 0, 5};                              // Expected: LLONG_MAX
    long long test5[] = {-10, 20, -5, 0, 1000};                            // Expected: 5

    struct {
        const long long* arr;
        size_t len;
    } tests[] = {
        {test1, sizeof(test1)/sizeof(test1[0])},
        {test2, sizeof(test2)/sizeof(test2[0])},
        {test3, sizeof(test3)/sizeof(test3[0])},
        {test4, sizeof(test4)/sizeof(test4[0])},
        {test5, sizeof(test5)/sizeof(test5[0])}
    };

    for (size_t t = 0; t < sizeof(tests)/sizeof(tests[0]); ++t) {
        long long result = totalFirstQuarterSales(tests[t].arr, tests[t].len);
        printf("Test %zu [%lld, %lld, %lld] => %lld\n",
               t + 1,
               tests[t].arr[0],
               tests[t].arr[1],
               tests[t].arr[2],
               result);
    }

    return 0;
}