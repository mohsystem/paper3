#include <stdio.h>
#include <math.h>

// Define M_PI if it's not already defined by math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Calculates the area of a circle securely.
 * 
 * @param radius The radius of the circle. Must be a non-negative number.
 * @return The area of the circle, or -1.0 if the radius is negative or not finite.
 */
double calculateCircleArea(double radius) {
    if (radius < 0 || !isfinite(radius)) {
        return -1.0; // Error code for invalid input
    }
    return M_PI * radius * radius;
}

int main() {
    double testRadii[] = {0.0, 1.0, 5.5, 10.0, -2.0};
    int numTests = sizeof(testRadii) / sizeof(testRadii[0]);

    printf("Running C test cases:\n");

    for (int i = 0; i < numTests; i++) {
        double radius = testRadii[i];
        double area = calculateCircleArea(radius);

        if (area == -1.0) {
            printf("Radius: %.2f, Error: Radius cannot be negative or non-finite.\n", radius);
        } else {
            printf("Radius: %.2f, Area: %.2f\n", radius, area);
        }
    }

    return 0;
}