#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <float.h>

static bool approx_equal(double a, double b, double eps) {
    return fabs(a - b) <= eps;
}

double findUniq(const double* arr, size_t n, double eps) {
    if (arr == NULL || n < 3) {
        return NAN;
    }

    double a0 = arr[0];
    double a1 = arr[1];
    double a2 = arr[2];

    double common = a0;
    if (approx_equal(a0, a1, eps) || approx_equal(a0, a2, eps)) {
        common = a0;
    } else {
        common = a1;
    }

    for (size_t i = 0; i < n; ++i) {
        if (!approx_equal(arr[i], common, eps)) {
            return arr[i];
        }
    }

    return NAN;
}

int main(void) {
    const double eps = 1e-9;

    // Test case 1
    double t1[] = {1, 1, 1, 2, 1, 1};
    printf("Unique (t1): %.12f\n", findUniq(t1, sizeof(t1)/sizeof(t1[0]), eps)); // Expected: 2

    // Test case 2
    double t2[] = {0, 0, 0.55, 0, 0};
    printf("Unique (t2): %.12f\n", findUniq(t2, sizeof(t2)/sizeof(t2[0]), eps)); // Expected: 0.55

    // Test case 3
    double t3[] = {3.3, 99.9, 3.3, 3.3, 3.3};
    printf("Unique (t3): %.12f\n", findUniq(t3, sizeof(t3)/sizeof(t3[0]), eps)); // Expected: 99.9

    // Test case 4
    double t4[] = {-5.0, -5.0, -5.0, -77.0, -5.0};
    printf("Unique (t4): %.12f\n", findUniq(t4, sizeof(t4)/sizeof(t4[0]), eps)); // Expected: -77.0

    // Test case 5
    double t5[] = {42.42, 42.42, 42.42, 42.42, 0.0};
    printf("Unique (t5): %.12f\n", findUniq(t5, sizeof(t5)/sizeof(t5[0]), eps)); // Expected: 0.0

    return 0;
}