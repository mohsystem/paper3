#include <iostream>
#include <cmath>
#include <iomanip>

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
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    // 5 test cases
    std::cout << std::fixed << std::setprecision(2);
    std::cout << cubeDiagonal(8) << std::endl;
    std::cout << cubeDiagonal(343) << std::endl;
    std::cout << cubeDiagonal(1157.625) << std::endl;
    std::cout << cubeDiagonal(1) << std::endl;
    std::cout << cubeDiagonal(27) << std::endl;
    return 0;
}