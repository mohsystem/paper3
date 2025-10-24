#include <iostream>
#include <cmath>
#include <iomanip>

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
    // Set output precision for floating point numbers
    std::cout << std::fixed << std::setprecision(2);

    // Test cases
    std::cout << cubeDiagonal(8) << std::endl;
    std::cout << cubeDiagonal(343) << std::endl;
    std::cout << cubeDiagonal(1157.625) << std::endl;
    std::cout << cubeDiagonal(1) << std::endl;
    std::cout << cubeDiagonal(1000) << std::endl;
    
    return 0;
}