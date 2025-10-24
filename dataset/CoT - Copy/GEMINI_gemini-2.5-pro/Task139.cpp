#include <iostream>
#include <cmath> // For M_PI and pow
#include <vector>
#include <iomanip> // For std::fixed and std::setprecision

// M_PI is not in the C++ standard, but defined in <cmath> by most compilers.
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Calculates the area of a circle.
 * 
 * @param radius The radius of the circle. Must be a non-negative number.
 * @return The area of the circle, or -1.0 if the radius is negative.
 */
double calculateCircleArea(double radius) {
    // Security: Ensure the radius is not negative.
    if (radius < 0) {
        return -1.0; // Error indicator for invalid input.
    }
    return M_PI * radius * radius;
}

int main() {
    // 5 Test Cases
    std::vector<double> testRadii = {10.0, 5.5, 0.0, 1.0, -7.0};
    
    std::cout << "CPP Test Cases:" << std::endl;
    for (double radius : testRadii) {
        double area = calculateCircleArea(radius);
        if (area == -1.0) {
            std::cout << "Radius: " << radius << " -> Error: Radius cannot be negative." << std::endl;
        } else {
            std::cout << std::fixed << std::setprecision(2)
                      << "Radius: " << radius << " -> Area: " << area << std::endl;
        }
    }
    return 0;
}