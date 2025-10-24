#include <stdio.h>
#include <math.h>

/**
 * Calculates the length of a cube's main diagonal from its volume.
 *
 * @param volume The volume of the cube.
 * @return The length of the main diagonal, rounded to two decimal places.
 */
double cubeDiagonal(double volume) {
    if (volume < 0) {
        return 0.0; // Or handle error appropriately
    }
    // side = cube root of volume
    double side = cbrt(volume);
    // diagonal = side * sqrt(3)
    double diagonal = side * sqrt(3.0);
    // Round to two decimal places
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    // Test cases
    printf("Test Case 1: cubeDiagonal(8)\n");
    printf("Expected: 3.46\n");
    printf("Actual: %.2f\n\n", cubeDiagonal(8));

    printf("Test Case 2: cubeDiagonal(343)\n");
    printf("Expected: 12.12\n");
    printf("Actual: %.2f\n\n", cubeDiagonal(343));

    printf("Test Case 3: cubeDiagonal(1157.625)\n");
    printf("Expected: 18.19\n");
    printf("Actual: %.2f\n\n", cubeDiagonal(1157.625));

    printf("Test Case 4: cubeDiagonal(1)\n");
    printf("Expected: 1.73\n");
    printf("Actual: %.2f\n\n", cubeDiagonal(1));

    printf("Test Case 5: cubeDiagonal(27)\n");
    printf("Expected: 5.20\n");
    printf("Actual: %.2f\n\n", cubeDiagonal(27));

    return 0;
}