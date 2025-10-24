#include <ctime>
#include <iostream>
#include <iomanip>
#include <limits>

static volatile unsigned long long G_SINK = 0ULL;

// Returns CPU time used by the process in seconds, or -1.0 on failure.
double getProcessorTimeSeconds() {
    std::clock_t c = std::clock();
    if (c == static_cast<std::clock_t>(-1)) {
        return -1.0;
    }
    return static_cast<double>(c) / static_cast<double>(CLOCKS_PER_SEC);
}

void doCpuWork(int n) {
    if (n <= 0) return;
    unsigned long long local = G_SINK;
    for (int i = 0; i < n; ++i) {
        unsigned long long ui = static_cast<unsigned long long>(static_cast<unsigned int>(i));
        local += (ui ^ (ui << 1));
    }
    G_SINK = local;
}

int main() {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        doCpuWork(200000 * i);
        double secs = getProcessorTimeSeconds();
        std::cout << "Test " << i << ": " << std::fixed << std::setprecision(6) << secs << " seconds\n";
    }
    return 0;
}