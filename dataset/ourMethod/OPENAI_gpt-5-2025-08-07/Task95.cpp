#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <ctime>

double getProcessorTimeSeconds(int mode) {
    if (mode != 0 && mode != 1) {
        throw std::invalid_argument("mode must be 0 or 1");
    }
    std::clock_t ticks = std::clock();
    if (ticks == static_cast<std::clock_t>(-1)) {
        return -1.0;
    }
    return static_cast<double>(ticks) / static_cast<double>(CLOCKS_PER_SEC);
}

double burn_cpu(long long iterations) {
    if (iterations < 0) {
        iterations = 0;
    }
    double x = 1.0;
    for (long long i = 0; i < iterations; ++i) {
        x = x * 1.0000001 + 0.0000001;
        if (x > 2.0e9) {
            x = 1.0;
        }
    }
    return x;
}

int main() {
    std::cout << std::fixed << std::setprecision(9);
    try {
        // 5 test cases
        std::cout << getProcessorTimeSeconds(0) << "\n";
        double sink = burn_cpu(2000000LL);
        std::cout << getProcessorTimeSeconds(1) << "\n";
        sink += burn_cpu(4000000LL);
        std::cout << getProcessorTimeSeconds(0) << "\n";
        sink += burn_cpu(1000000LL);
        std::cout << getProcessorTimeSeconds(1) << "\n";
        if (sink == -1.0) { std::cout << ""; }
        std::cout << getProcessorTimeSeconds(0) << "\n";
    } catch (const std::exception& ex) {
        // Fail closed: no additional actions, but ensure program ends predictably
        return 1;
    }
    return 0;
}