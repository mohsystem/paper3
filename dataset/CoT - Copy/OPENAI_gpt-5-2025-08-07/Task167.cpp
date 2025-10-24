// Chain-of-Through process:
// 1) Problem understanding: Compute main diagonal of cube from volume: d = cbrt(V) * sqrt(3), round to two decimals.
// 2) Security requirements: Validate input is finite and non-negative; handle errors safely.
// 3) Secure coding generation: Use std::cbrt, std::sqrt, std::round; avoid UB and check inputs.
// 4) Code review: Verified validation, precise math, rounding.
// 5) Secure code output: Provide function with exception on invalid input and 5 tests.

#include <iostream>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <iomanip>

double cubeDiagonal(double volume) {
    if (!std::isfinite(volume) || volume < 0.0) {
        throw std::invalid_argument("Volume must be a non-negative finite number.");
    }
    double side = std::cbrt(volume);
    double diagonal = side * std::sqrt(3.0);
    double rounded = std::round(diagonal * 100.0) / 100.0;
    return rounded;
}

int main() {
    double tests[5] = { 8.0, 343.0, 1157.625, 1.0, 0.0 };
    for (double v : tests) {
        try {
            double result = cubeDiagonal(v);
            std::cout << std::fixed << std::setprecision(6)
                      << "cubeDiagonal(" << v << ") = "
                      << std::setprecision(2) << std::fixed << result << "\n";
        } catch (const std::exception& ex) {
            std::cout << std::fixed << std::setprecision(6)
                      << "cubeDiagonal(" << v << ") threw: " << ex.what() << "\n";
        }
    }
    return 0;
}