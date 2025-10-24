#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <iomanip>

double cubeDiagonal(double volume) {
    if (!std::isfinite(volume) || volume < 0.0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    double side = std::cbrt(volume);
    double diag = side * std::sqrt(3.0);
    double rounded = std::round(diag * 100.0) / 100.0;
    return rounded;
}

int main() {
    std::vector<double> tests = {8.0, 343.0, 1157.625, 0.0, 1.0};
    std::cout << std::fixed << std::setprecision(2);
    for (double v : tests) {
        double result = cubeDiagonal(v);
        std::cout << "cubeDiagonal(" << std::setprecision(6) << v << std::setprecision(2)
                  << ") = " << result << "\n";
    }
    return 0;
}