#include <stdio.h>
#include <math.h> // For M_PI and pow

// M_PI is a non-standard macro, but common. Define if not available.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * Calculates the area of a circle.
 *
 * @param radius The radius of the circle. Must be a non-negative number.
 * @return The area of the circle, or -1.0 if the radius is negative.
 */
double calculateCircleArea(double radius) {
    // Security: Ensure the radius is not negative.
    if (radius < 0) {
        return -1.0; // Return an error indicator for invalid input.
    }
    return M_PI * radius * radius;
}

int main() {
    // 5 Test Cases
    double testRadii[] = {10.0, 5.5, 0.0, 1.0, -7.0};
    int num_tests = sizeof(testRadii) / sizeof(testRadii[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < num_tests; i++) {
        double radius = testRadii[i];
        double area = calculateCircleArea(radius);
        if (area == -1.0) {
            printf("Radius: %.2f -> Error: Radius cannot be negative.\n", radius);
        } else {
            printf("Radius: %.2f -> Area: %.2f\n", radius, area);
        }
    }
    return 0;
}