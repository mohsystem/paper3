#include <iostream>
#include <cmath>
#include <stdexcept>
#include <iomanip>

/**
 * @brief Calculates the length of a cube's main diagonal given its volume.
 * 
 * @param volume The volume of the cube. Must be a positive number.
 * @return The length of the main diagonal, rounded to two decimal places.
 * @throw std::invalid_argument if the volume is not a positive number.
 */
double cubeDiagonal(double volume) {
    if (volume <= 0) {
        throw std::invalid_argument("Volume must be a positive number.");
    }
    double side = std::cbrt(volume);
    double diagonal = side * std::sqrt(3.0);
    return std::round(diagonal * 100.0) / 100.0;
}

int main() {
    // Set output to two decimal places for printing
    std::cout << std::fixed << std::setprecision(2);

    // Test Case 1
    std::cout << cubeDiagonal(8) << std::endl;

    // Test Case 2
    std::cout << cubeDiagonal(343) << std::endl;

    // Test Case 3
    std::cout << cubeDiagonal(1157.625) << std::endl;

    // Test Case 4: Unit cube
    std::cout << cubeDiagonal(1) << std::endl;

    // Test Case 5: Invalid input
    try {
        std::cout << cubeDiagonal(-1) << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}