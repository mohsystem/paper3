#include <stdio.h>
#include <math.h>

// M_PI is non-standard, so define it if it's not already defined.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Calculates the area of a circle.
 *
 * @param radius The radius of the circle.
 * @return The area of the circle.
 */
double calculateArea(double radius) {
    return M_PI * radius * radius;
}

int main() {
    // 5 Test Cases
    double testRadii[] = {1.0, 5.0, 10.0, 0.0, 7.5};
    int num_tests = sizeof(testRadii) / sizeof(testRadii[0]);
    
    printf("C Results:\n");
    for (int i = 0; i < num_tests; i++) {
        double radius = testRadii[i];
        printf("Radius: %f, Area: %f\n", radius, calculateArea(radius));
    }
    return 0;
}