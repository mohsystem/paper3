#include <iostream>
#include <vector>
#include <cmath>

static const double EPS = 1e-9;

static bool eq(double a, double b) {
    return std::fabs(a - b) <= EPS;
}

double findUniq(const std::vector<double>& a) {
    if (a.size() < 3) {
        throw std::invalid_argument("Array must have at least 3 elements");
    }
    double majority;
    if (eq(a[0], a[1]) || eq(a[0], a[2])) {
        majority = a[0];
    } else {
        majority = a[1];
    }
    for (double v : a) {
        if (!eq(v, majority)) {
            return v;
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

int main() {
    std::vector<std::vector<double>> tests = {
        {1, 1, 1, 2, 1, 1},
        {0, 0, 0.55, 0, 0},
        {-2, -2, -2, -3, -2},
        {5, 6, 5, 5, 5},
        {7.7, 7.7, 7.7, 7.7, 8.8}
    };

    for (const auto& t : tests) {
        std::cout << findUniq(t) << std::endl;
    }
    return 0;
}