#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>

static double getMonthlySales(const std::vector<double>& sales, std::size_t i) {
    if (i >= sales.size()) {
        return 0.0;
    }
    double v = sales[i];
    if (!std::isfinite(v)) {
        return 0.0;
    }
    return v;
}

static double totalFirstQuarter(const std::vector<double>& sales) {
    double sum = 0.0;
    for (std::size_t i = 0; i < 3; ++i) {
        sum += getMonthlySales(sales, i);
    }
    return sum;
}

int main() {
    std::vector<std::vector<double>> tests = {
        {100.0, 200.0, 300.0},
        {50.5, 60.2, 70.3},
        {1000.0, 2000.0, 3000.0, 4000.0},
        {10.0},
        {-5.0, 15.0, 25.0}
    };

    std::cout << std::fixed << std::setprecision(2);
    for (std::size_t t = 0; t < tests.size(); ++t) {
        double res = totalFirstQuarter(tests[t]);
        std::cout << "Test " << (t + 1) << " -> Q1 total=" << res << "\n";
    }
    return 0;
}