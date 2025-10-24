#include <stdio.h>
#include <math.h>

/**
 * @brief Calculates the length of a cube's main diagonal from its volume.
 * 
 * @param volume The volume of the cube.
 * @return The length of the main diagonal, rounded to two decimal places.
 */
double cubeDiagonal(double volume) {
    // Side of the cube = cube root of the volume
    double side = cbrt(volume);
    
    // Main diagonal = side * sqrt(3)
    double diagonal = side * sqrt(3);
    
    // Round the result to two decimal places
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    // Test cases
    printf("%.2f\n", cubeDiagonal(8));
    printf("%.2f\n", cubeDiagonal(343));
    printf("%.2f\n", cubeDiagonal(1157.625));
    printf("%.2f\n", cubeDiagonal(1));
    printf("%.2f\n", cubeDiagonal(1000));
    
    return 0;
}