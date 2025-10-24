#include <iostream>
#include <cmath>
#include <limits>
#include <vector>
#include <iomanip>

class Task167 {
public:
    static double cubeDiagonal(double volume) {
        if (!std::isfinite(volume) || volume < 0.0) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        long double diag = std::cbrt(volume) * std::sqrt(3.0L);
        long double rounded = std::round(diag * 100.0L) / 100.0L;
        return static_cast<double>(rounded);
    }
};

int main() {
    std::vector<double> tests = {8.0, 343.0, 1157.625, 0.0, -5.0};
    for (double v : tests) {
        double res = Task167::cubeDiagonal(v);
        std::cout << "cubeDiagonal(" << v << ") = ";
        if (std::isnan(res)) {
            std::cout << "NaN";
        } else {
            std::cout << std::fixed << std::setprecision(2) << res;
        }
        std::cout << std::endl;
    }
    return 0;
}