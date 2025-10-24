#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <cmath>

static constexpr double PI = 3.141592653589793238462643383279502884;
static const double MAX_SAFE_RADIUS = std::sqrt(std::numeric_limits<double>::max() / PI);

double computeCircleArea(double radius) {
    if (!std::isfinite(radius)) {
        throw std::invalid_argument("Radius must be a finite number.");
    }
    if (radius < 0.0) {
        throw std::invalid_argument("Radius must be non-negative.");
    }
    if (radius > MAX_SAFE_RADIUS) {
        throw std::invalid_argument("Radius too large; area would overflow.");
    }
    return PI * radius * radius;
}

int main() {
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(6);

    double tests[5] = {0.0, 1.0, 2.5, 1e150, -3.0};
    for (double r : tests) {
        try {
            double area = computeCircleArea(r);
            std::cout << "radius=" << r << " -> area=" << area << "\n";
        } catch (const std::exception& ex) {
            std::cout << "radius=" << r << " -> error=" << ex.what() << "\n";
        }
    }
    return 0;
}