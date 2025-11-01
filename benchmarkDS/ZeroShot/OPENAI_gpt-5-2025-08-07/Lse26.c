#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>

int getMonthlySales(const double* sales, size_t len, size_t i, double* out) {
    if (sales == NULL || out == NULL) {
        return -1; /* invalid pointers */
    }
    if (i >= len) {
        return -2; /* index out of bounds */
    }
    double val = sales[i];
    if (!isfinite(val)) {
        return -3; /* non-finite value */
    }
    *out = val;
    return 0;
}

int computeFirstQuarterSales(const double* sales, size_t len, double* out) {
    if (sales == NULL || out == NULL) {
        return -1; /* invalid pointers */
    }
    if (len < 3) {
        return -4; /* insufficient elements */
    }
    double sum = 0.0;
    for (size_t i = 0; i <= 2; ++i) {
        double val = 0.0;
        int rc = getMonthlySales(sales, len, i, &val);
        if (rc != 0) {
            return rc;
        }
        sum += val;
    }
    *out = sum;
    return 0;
}

int main(void) {
    double tc1[] = {120.5, 340.75, 230.25};
    double tc2[] = {100.0, 0.0, 300.0, 400.0};
    double tc3[] = {0.0, 0.0, 0.0};
    double tc4[] = {1e6, 2e6, 3e6};
    double tc5[] = {500.0, -100.0, 50.0};

    const struct {
        const double* arr;
        size_t len;
    } tests[] = {
        {tc1, sizeof(tc1)/sizeof(tc1[0])},
        {tc2, sizeof(tc2)/sizeof(tc2[0])},
        {tc3, sizeof(tc3)/sizeof(tc3[0])},
        {tc4, sizeof(tc4)/sizeof(tc4[0])},
        {tc5, sizeof(tc5)/sizeof(tc5[0])}
    };

    for (size_t t = 0; t < sizeof(tests)/sizeof(tests[0]); ++t) {
        double result = 0.0;
        int rc = computeFirstQuarterSales(tests[t].arr, tests[t].len, &result);
        if (rc == 0) {
            printf("Test %zu => Q1 total=%.2f\n", t + 1, result);
        } else {
            printf("Test %zu => Error code: %d\n", t + 1, rc);
        }
    }

    return 0;
}