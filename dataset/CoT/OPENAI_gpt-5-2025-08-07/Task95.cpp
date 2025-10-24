// Chain-of-Through Step 1: Problem understanding
// - Provide a function returning CPU time in seconds and show 5 test cases.
//
// Chain-of-Through Step 2: Security requirements
// - No external input; use safe standard library functions.
//
// Chain-of-Through Step 3: Secure coding generation
// - Use std::clock() / CLOCKS_PER_SEC for process CPU time.
//
// Chain-of-Through Step 4: Code review
// - No undefined behavior; careful with types and printing.
//
// Chain-of-Through Step 5: Secure code output
// - Final code below.

#include <ctime>
#include <cmath>
#include <iostream>
#include <iomanip>

double getProcessorTimeSeconds() {
    std::clock_t c = std::clock();
    if (c == (std::clock_t)-1) {
        return -1.0; // indicate unavailability
    }
    return static_cast<double>(c) / static_cast<double>(CLOCKS_PER_SEC);
}

volatile double cpp_sink = 0.0;
void burnCpu(long long iterations) {
    double x = 0.0;
    for (long long i = 1; i <= iterations; ++i) {
        x += std::sqrt(static_cast<double>((i % 1000) + 1));
    }
    cpp_sink = x; // prevent optimization
}

int main() {
    for (int t = 1; t <= 5; ++t) {
        burnCpu(200000LL * t);
        double seconds = getProcessorTimeSeconds();
        std::cout << "C++ Test " << t << " - CPU Time (s): "
                  << std::fixed << std::setprecision(9) << seconds << "\n";
    }
    return 0;
}