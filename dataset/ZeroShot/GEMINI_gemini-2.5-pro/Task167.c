#include <stdio.h>
#include <math.h>

/**
 * @brief Calculates the length of a cube's main diagonal from its volume.
 * 
 * @param volume The volume of the cube. Must be a positive number.
 * @return The length of the main diagonal, rounded to two decimal places.
 *         Returns 0.0 for non-positive volume.
 */
double cubeDiagonal(double volume) {
    // Secure coding: Validate input to ensure it's within the valid domain.
    if (volume <= 0) {
        return 0.0;
    }
    
    // V = s^3  =>  s = cbrt(V)
    double side = cbrt(volume);
    
    // Diagonal d = s * sqrt(3)
    double diagonal = side * sqrt(3);
    
    // Round the result to two decimal places.
    // round() is from <math.h> in C99 and later.
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    // 5 test cases
    printf("%.2f\n", cubeDiagonal(8));
    printf("%.2f\n", cubeDiagonal(343));
    printf("%.2f\n", cubeDiagonal(1157.625));
    printf("%.2f\n", cubeDiagonal(1));
    printf("%.2f\n", cubeDiagonal(27));
    return 0;
}