#include <iostream>
#include <vector>
#include <cmath>
#include <stdexcept>

static bool equal_double(double x, double y) {
    return (std::isnan(x) && std::isnan(y)) || x == y;
}

double findUniq(const std::vector<double>& arr) {
    if (arr.size() < 3) {
        throw std::invalid_argument("Array must contain at least 3 numbers.");
    }

    double a = arr[0], b = arr[1], c = arr[2];
    double majority;
    if (equal_double(a, b) || equal_double(a, c)) {
        majority = a;
    } else {
        majority = b; // since a != b and a != c, b == c is the majority
    }

    for (double v : arr) {
        if (!equal_double(v, majority)) {
            return v;
        }
    }

    throw std::invalid_argument("No unique element found.");
}

int main() {
    std::vector<std::vector<double>> tests = {
        {1, 1, 1, 2, 1, 1},
        {0, 0, 0.55, 0, 0},
        {3, 4, 4, 4, 4},
        {-1, -1, -1, -2, -1, -1},
        {std::nan(""), std::nan(""), 7.0, std::nan("")}
    };

    for (const auto& t : tests) {
        try {
            std::cout << findUniq(t) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}