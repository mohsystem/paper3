#include <ctime>
#include <iostream>

double getProcessorTimeSeconds() {
    return static_cast<double>(std::clock()) / static_cast<double>(CLOCKS_PER_SEC);
}

volatile unsigned long sink_cpp = 0;
unsigned long burnCpu(unsigned long n) {
    unsigned long s = 0;
    for (unsigned long i = 0; i < n; ++i) {
        s += i % 7;
    }
    sink_cpp = s;
    return s;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        double secs = getProcessorTimeSeconds();
        std::cout.setf(std::ios::fixed);
        std::cout.precision(9);
        std::cout << "Run " << i << " - CPU time (s): " << secs << "\n";
        burnCpu(50000000UL);
    }
    return 0;
}