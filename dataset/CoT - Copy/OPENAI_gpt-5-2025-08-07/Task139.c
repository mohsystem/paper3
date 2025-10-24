// Step 1-5: Secure implementation to compute area of a circle from radius.
#include <stdio.h>
#include <math.h>
#include <float.h>

static const double PI = 3.14159265358979323846;

double compute_area(double radius) {
    // Validate input: finite and non-negative
    if (!isfinite(radius) || radius < 0.0) {
        return NAN;
    }
    // Prevent overflow for extremely large radii
    double limit = sqrt(DBL_MAX / PI);
    if (fabs(radius) > limit) {
        return INFINITY;
    }
    return PI * radius * radius;
}

// 5 test cases
int main(void) {
    double tests[5] = {0.0, 1.0, 2.5, 10.0, 12345.678};
    for (int i = 0; i < 5; ++i) {
        double r = tests[i];
        double area = compute_area(r);
        printf("radius=%.6f -> area=%.15f\n", r, area);
    }
    return 0;
}