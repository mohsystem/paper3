#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <iomanip>

double getMonthlySales(const std::vector<double>& sales, std::size_t i) {
    if (i >= sales.size()) {
        throw std::out_of_range("Index out of bounds: " + std::to_string(i));
    }
    double val = sales[i];
    if (!std::isfinite(val)) {
        throw std::invalid_argument("Sales value must be finite for index: " + std::to_string(i));
    }
    return val;
}

double computeFirstQuarterSales(const std::vector<double>& sales) {
    if (sales.size() < 3) {
        throw std::invalid_argument("Sales vector must contain at least 3 elements.");
    }
    double sum = 0.0;
    for (std::size_t i = 0; i <= 2; ++i) {
        sum += getMonthlySales(sales, i);
    }
    return sum;
}

int main() {
    std::vector<std::vector<double>> testCases = {
        {120.5, 340.75, 230.25},
        {100.0, 0.0, 300.0, 400.0},
        {0.0, 0.0, 0.0},
        {1e6, 2e6, 3e6},
        {500.0, -100.0, 50.0}
    };

    std::cout << std::fixed << std::setprecision(2);
    for (std::size_t t = 0; t < testCases.size(); ++t) {
        try {
            double result = computeFirstQuarterSales(testCases[t]);
            std::cout << "Test " << (t + 1) << " => Q1 total=" << result << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (t + 1) << " => Error: " << ex.what() << "\n";
        }
    }
    return 0;
}