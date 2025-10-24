#include <stdio.h>
#include <math.h>

// M_PI is a non-standard extension, define it if it's not provided by math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Calculates the area of a circle given its radius.
 * The area is calculated as PI * radius^2.
 *
 * @param radius The radius of the circle. Must be a non-negative number.
 * @return The area of the circle, or -1.0 if the radius is negative.
 */
double calculateCircleArea(double radius) {
    if (radius < 0) {
        return -1.0; // Return a sentinel value to indicate an error
    }
    return M_PI * radius * radius;
}

int main() {
    double testRadii[] = {0.0, 1.0, 5.5, 10.0, -2.0};
    int numTests = sizeof(testRadii) / sizeof(testRadii[0]);

    for (int i = 0; i < numTests; i++) {
        double area = calculateCircleArea(testRadii[i]);
        if (area < 0) {
            printf("Test Case %d: Radius = %.2f, Error: Radius cannot be negative.\n", i + 1, testRadii[i]);
        } else {
            printf("Test Case %d: Radius = %.2f, Area = %.4f\n", i + 1, testRadii[i], area);
        }
    }

    return 0;
}