#include <stdio.h>
#include <math.h>
#include <float.h>

double compute_circle_area(double radius) {
    if (!isfinite(radius) || radius < 0.0) {
        return NAN;
    }
    const double PI = 3.141592653589793238462643383279502884;
    double threshold = sqrt(DBL_MAX / PI);
    if (radius > threshold) {
        return INFINITY;
    }
    return PI * radius * radius;
}

int main(void) {
    double tests[5] = {0.0, 1.0, 2.5, 1e154, -3.0};
    for (int i = 0; i < 5; ++i) {
        double r = tests[i];
        double area = compute_circle_area(r);
        if (isnan(area)) {
            printf("radius=%g error=invalid radius\n", r);
        } else {
            printf("radius=%.17g area=%.17g\n", r, area);
        }
    }
    return 0;
}