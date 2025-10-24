#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <stdexcept>
#include <iomanip>

double computeCircleArea(double radius) {
    if (!std::isfinite(radius)) {
        throw std::invalid_argument("Radius must be a finite number.");
    }
    if (radius < 0.0) {
        throw std::invalid_argument("Radius must be non-negative.");
    }
    const double PI = std::acos(-1.0);
    const double threshold = std::sqrt(std::numeric_limits<double>::max() / PI);
    if (radius > threshold) {
        return std::numeric_limits<double>::infinity();
    }
    return PI * radius * radius;
}

int main() {
    std::vector<double> tests = {0.0, 1.0, 2.5, 1e154, -3.0};
    for (double r : tests) {
        try {
            double area = computeCircleArea(r);
            std::cout << "radius=" << std::setprecision(17) << r << " area=" << area << std::endl;
        } catch (const std::exception& ex) {
            std::cout << "radius=" << r << " error=" << ex.what() << std::endl;
        }
    }
    return 0;
}