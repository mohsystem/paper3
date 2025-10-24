#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <iomanip>

static bool approx_equal(double a, double b, double eps) {
    return std::fabs(a - b) <= eps;
}

double findUniq(const std::vector<double>& arr, double eps) {
    if (arr.size() < 3) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double a0 = arr[0];
    const double a1 = arr[1];
    const double a2 = arr[2];

    double common = a0;
    if (approx_equal(a0, a1, eps) || approx_equal(a0, a2, eps)) {
        common = a0;
    } else {
        common = a1;
    }

    for (size_t i = 0; i < arr.size(); ++i) {
        if (!approx_equal(arr[i], common, eps)) {
            return arr[i];
        }
    }

    return std::numeric_limits<double>::quiet_NaN();
}

int main() {
    std::cout << std::fixed << std::setprecision(12);

    // Test case 1
    std::vector<double> t1{1, 1, 1, 2, 1, 1};
    std::cout << "Unique (t1): " << findUniq(t1, 1e-9) << "\n"; // Expected: 2

    // Test case 2
    std::vector<double> t2{0, 0, 0.55, 0, 0};
    std::cout << "Unique (t2): " << findUniq(t2, 1e-9) << "\n"; // Expected: 0.55

    // Test case 3
    std::vector<double> t3{3.3, 99.9, 3.3, 3.3, 3.3};
    std::cout << "Unique (t3): " << findUniq(t3, 1e-9) << "\n"; // Expected: 99.9

    // Test case 4
    std::vector<double> t4{-5.0, -5.0, -5.0, -77.0, -5.0};
    std::cout << "Unique (t4): " << findUniq(t4, 1e-9) << "\n"; // Expected: -77.0

    // Test case 5
    std::vector<double> t5{42.42, 42.42, 42.42, 42.42, 0.0};
    std::cout << "Unique (t5): " << findUniq(t5, 1e-9) << "\n"; // Expected: 0.0

    return 0;
}