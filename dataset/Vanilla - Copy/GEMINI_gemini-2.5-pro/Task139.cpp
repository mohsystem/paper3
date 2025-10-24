#include <iostream>
#include <cmath>
#include <vector>

/**
 * @brief Calculates the area of a circle.
 *
 * @param radius The radius of the circle.
 * @return The area of the circle.
 */
double calculateArea(double radius) {
    // M_PI is non-standard, acos(-1.0) is a portable way to get PI
    const double PI = acos(-1.0);
    return PI * radius * radius;
}

int main() {
    // 5 Test Cases
    std::vector<double> testRadii = {1.0, 5.0, 10.0, 0.0, 7.5};
    std::cout << "CPP Results:" << std::endl;
    for (double radius : testRadii) {
        std::cout << "Radius: " << radius << ", Area: " << calculateArea(radius) << std::endl;
    }
    return 0;
}