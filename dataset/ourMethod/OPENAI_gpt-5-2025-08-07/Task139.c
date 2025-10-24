#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>

typedef struct {
    int ok;           /* 1 = success, 0 = failure */
    double area;      /* valid if ok == 1 */
    char error[64];   /* error message if ok == 0 */
} AreaResult;

static double get_max_safe_radius(void) {
    const double PI = 3.14159265358979323846;
    return sqrt(DBL_MAX / PI);
}

AreaResult compute_circle_area(double radius) {
    AreaResult res;
    res.ok = 0;
    res.area = 0.0;
    res.error[0] = '\0';

    if (!isfinite(radius)) {
        snprintf(res.error, sizeof(res.error), "Radius must be a finite number.");
        return res;
    }
    if (radius < 0.0) {
        snprintf(res.error, sizeof(res.error), "Radius must be non-negative.");
        return res;
    }
    const double max_safe = get_max_safe_radius();
    if (radius > max_safe) {
        snprintf(res.error, sizeof(res.error), "Radius too large; area would overflow.");
        return res;
    }

    const double PI = 3.14159265358979323846;
    res.area = PI * radius * radius;
    res.ok = 1;
    return res;
}

int main(void) {
    double tests[5] = {0.0, 1.0, 2.5, 1e150, -3.0};
    for (size_t i = 0; i < 5; i++) {
        double r = tests[i];
        AreaResult res = compute_circle_area(r);
        if (res.ok) {
            printf("radius=%.6f -> area=%.6f\n", r, res.area);
        } else {
            printf("radius=%.6f -> error=%s\n", r, res.error);
        }
    }
    return 0;
}