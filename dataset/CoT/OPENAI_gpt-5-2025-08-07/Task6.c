#include <stdio.h>
#include <math.h>
#include <stddef.h>
#include <stdbool.h>

#define EPS 1e-9

static bool eq(double a, double b) {
    return fabs(a - b) <= EPS;
}

double findUniq(const double* a, size_t n) {
    if (a == NULL || n < 3) {
        /* Invalid input; for safety return NaN */
        return NAN;
    }
    double majority;
    if (eq(a[0], a[1]) || eq(a[0], a[2])) {
        majority = a[0];
    } else {
        majority = a[1];
    }
    for (size_t i = 0; i < n; ++i) {
        if (!eq(a[i], majority)) {
            return a[i];
        }
    }
    return NAN;
}

int main(void) {
    double t1[] = {1, 1, 1, 2, 1, 1};
    double t2[] = {0, 0, 0.55, 0, 0};
    double t3[] = {-2, -2, -2, -3, -2};
    double t4[] = {5, 6, 5, 5, 5};
    double t5[] = {7.7, 7.7, 7.7, 7.7, 8.8};

    printf("%.10g\n", findUniq(t1, sizeof(t1)/sizeof(t1[0])));
    printf("%.10g\n", findUniq(t2, sizeof(t2)/sizeof(t2[0])));
    printf("%.10g\n", findUniq(t3, sizeof(t3)/sizeof(t3[0])));
    printf("%.10g\n", findUniq(t4, sizeof(t4)/sizeof(t4[0])));
    printf("%.10g\n", findUniq(t5, sizeof(t5)/sizeof(t5[0])));

    return 0;
}