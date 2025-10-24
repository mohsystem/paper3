#include <iostream>
#include <cmath>
#include <vector>
#include <stdexcept>
#include <iomanip>

// Define M_PI if it's not already defined by cmath
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Calculates the area of a circle securely.
 * 
 * @param radius The radius of the circle. Must be a non-negative number.
 * @return The area of the circle.
 * @throws std::invalid_argument if the radius is negative or not a finite number.
 */
double calculateCircleArea(double radius) {
    if (radius < 0) {
        throw std::invalid_argument("Radius cannot be negative.");
    }
    if (!std::isfinite(radius)) {
        throw std::invalid_argument("Radius must be a finite number.");
    }
    return M_PI * radius * radius;
}

int main() {
    std::vector<double> testRadii = {0.0, 1.0, 5.5, 10.0, -2.0};
    std::cout << "Running C++ test cases:" << std::endl;

    for (const double& radius : testRadii) {
        try {
            double area = calculateCircleArea(radius);
            std::cout << std::fixed << std::setprecision(2)
                      << "Radius: " << radius << ", Area: " << area << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << std::fixed << std::setprecision(2)
                      << "Radius: " << radius << ", Error: " << e.what() << std::endl;
        }
    }
    return 0;
}