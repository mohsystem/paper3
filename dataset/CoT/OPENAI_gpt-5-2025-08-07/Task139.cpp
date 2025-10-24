// Step 1-5: Secure implementation to compute area of a circle from radius.
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

static const double PI = 3.141592653589793238462643383279502884;

double computeArea(double radius) {
    // Validate input: finite and non-negative
    if (!std::isfinite(radius) || radius < 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    // Prevent overflow for extremely large radii
    double limit = std::sqrt(std::numeric_limits<double>::max() / PI);
    if (std::fabs(radius) > limit) {
        return std::numeric_limits<double>::infinity();
    }
    return PI * radius * radius;
}

// 5 test cases
int main() {
    double tests[] = {0.0, 1.0, 2.5, 10.0, 12345.678};
    std::cout << std::setprecision(15) << std::fixed;
    for (double r : tests) {
        double area = computeArea(r);
        std::cout << "radius=" << std::setprecision(6) << r
                  << " -> area=" << std::setprecision(15) << area << "\n";
    }
    return 0;
}