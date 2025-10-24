#include <stdio.h>
#include <math.h>

/**
 * @brief Calculates the length of a cube's main diagonal given its volume.
 * 
 * @param volume The volume of the cube. Must be a positive number.
 * @return The length of the main diagonal, rounded to two decimal places.
 *         Returns -1.0 on error (e.g., non-positive volume).
 */
double cubeDiagonal(double volume) {
    if (volume <= 0) {
        // Return an error code since C doesn't have exceptions
        return -1.0; 
    }
    double side = cbrt(volume);
    double diagonal = side * sqrt(3.0);
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    double result;

    // Test Case 1
    result = cubeDiagonal(8);
    printf("%.2f\n", result);

    // Test Case 2
    result = cubeDiagonal(343);
    printf("%.2f\n", result);

    // Test Case 3
    result = cubeDiagonal(1157.625);
    printf("%.2f\n", result);

    // Test Case 4: Unit cube
    result = cubeDiagonal(1);
    printf("%.2f\n", result);

    // Test Case 5: Invalid input
    result = cubeDiagonal(-1);
    if (result < 0) {
        printf("Error: Volume must be a positive number.\n");
    } else {
        printf("%.2f\n", result);
    }

    return 0;
}