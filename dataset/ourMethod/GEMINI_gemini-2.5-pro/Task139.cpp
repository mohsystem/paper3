#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <iomanip>

// M_PI is not part of the C++ standard, define it if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief Calculates the area of a circle given its radius.
 * The area is calculated as PI * radius^2.
 * 
 * @param radius The radius of the circle. Must be a non-negative number.
 * @return The area of the circle.
 * @throw std::invalid_argument if the radius is negative.
 */
double calculateCircleArea(double radius) {
    if (radius < 0) {
        throw std::invalid_argument("Radius cannot be negative.");
    }
    return M_PI * radius * radius;
}

int main() {
    std::vector<double> testRadii = {0.0, 1.0, 5.5, 10.0, -2.0};

    for (size_t i = 0; i < testRadii.size(); ++i) {
        try {
            double area = calculateCircleArea(testRadii[i]);
            std::cout << "Test Case " << i + 1 << ": Radius = " << std::fixed << std::setprecision(2) << testRadii[i]
                      << ", Area = " << std::setprecision(4) << area << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Test Case " << i + 1 << ": Radius = " << std::fixed << std::setprecision(2) << testRadii[i]
                      << ", Error: " << e.what() << std::endl;
        }
    }
    return 0;
}