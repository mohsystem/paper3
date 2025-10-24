#include <iostream>
#include <cmath>
#include <iomanip>

/**
 * Calculates the length of a cube's main diagonal from its volume.
 *
 * @param volume The volume of the cube.
 * @return The length of the main diagonal, rounded to two decimal places.
 */
double cubeDiagonal(double volume) {
    if (volume < 0) {
        return 0.0; // Or throw an exception for invalid input
    }
    // side = cube root of volume
    double side = std::cbrt(volume);
    // diagonal = side * sqrt(3)
    double diagonal = side * std::sqrt(3);
    // Round to two decimal places
    return std::round(diagonal * 100.0) / 100.0;
}

int main() {
    // Test cases
    std::cout << std::fixed << std::setprecision(2);

    std::cout << "Test Case 1: cubeDiagonal(8)" << std::endl;
    std::cout << "Expected: 3.46" << std::endl;
    std::cout << "Actual: " << cubeDiagonal(8) << std::endl;
    std::cout << std::endl;

    std::cout << "Test Case 2: cubeDiagonal(343)" << std::endl;
    std::cout << "Expected: 12.12" << std::endl;
    std::cout << "Actual: " << cubeDiagonal(343) << std::endl;
    std::cout << std::endl;

    std::cout << "Test Case 3: cubeDiagonal(1157.625)" << std::endl;
    std::cout << "Expected: 18.19" << std::endl;
    std::cout << "Actual: " << cubeDiagonal(1157.625) << std::endl;
    std::cout << std::endl;

    std::cout << "Test Case 4: cubeDiagonal(1)" << std::endl;
    std::cout << "Expected: 1.73" << std::endl;
    std::cout << "Actual: " << cubeDiagonal(1) << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test Case 5: cubeDiagonal(27)" << std::endl;
    std::cout << "Expected: 5.20" << std::endl;
    std::cout << "Actual: " << cubeDiagonal(27) << std::endl;
    std::cout << std::endl;

    return 0;
}