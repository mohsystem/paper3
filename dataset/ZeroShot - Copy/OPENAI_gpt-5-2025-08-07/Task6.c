#include <stdio.h>
#include <math.h>
#include <stddef.h>

static int equal_double(double x, double y) {
    return ((isnan(x) && isnan(y)) || x == y);
}

double findUniq(const double* arr, size_t n) {
    if (arr == NULL || n < 3) {
        // Return NaN to indicate error for invalid input
        return NAN;
    }

    double a = arr[0], b = arr[1], c = arr[2];
    double majority;
    if (equal_double(a, b) || equal_double(a, c)) {
        majority = a;
    } else {
        majority = b; // since a != b and a != c, b == c is the majority
    }

    for (size_t i = 0; i < n; ++i) {
        if (!equal_double(arr[i], majority)) {
            return arr[i];
        }
    }

    return NAN; // Should not occur if constraints are met
}

int main(void) {
    double a1[] = {1, 1, 1, 2, 1, 1};
    double a2[] = {0, 0, 0.55, 0, 0};
    double a3[] = {3, 4, 4, 4, 4};
    double a4[] = {-1, -1, -1, -2, -1, -1};
    double a5[] = {NAN, NAN, 7.0, NAN};

    printf("%.17g\n", findUniq(a1, sizeof(a1)/sizeof(a1[0])));
    printf("%.17g\n", findUniq(a2, sizeof(a2)/sizeof(a2[0])));
    printf("%.17g\n", findUniq(a3, sizeof(a3)/sizeof(a3[0])));
    printf("%.17g\n", findUniq(a4, sizeof(a4)/sizeof(a4[0])));
    printf("%.17g\n", findUniq(a5, sizeof(a5)/sizeof(a5[0])));

    return 0;
}