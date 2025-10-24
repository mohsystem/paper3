#include <iostream>
#include <cmath>
#include <limits>

int nb_year(long long p0, double percent, long long aug, long long p) {
    // Validate inputs
    if (p0 <= 0 || p <= 0 || percent < 0.0 || !std::isfinite(percent)) {
        return -1;
    }
    long long cur = p0;
    if (cur >= p) return 0;

    const int maxYears = 10000000; // Safety cap
    int years = 0;

    while (cur < p) {
        if (years >= maxYears) return -1;

        // Compute floor(cur + cur * percent/100.0) safely
        double growth = static_cast<double>(cur) * (percent / 100.0);
        if (!std::isfinite(growth)) return -1;

        double baseD = std::floor(static_cast<double>(cur) + growth);
        if (!std::isfinite(baseD)) return -1;

        if (baseD > static_cast<double>(std::numeric_limits<long long>::max()) ||
            baseD < static_cast<double>(std::numeric_limits<long long>::min())) {
            return -1;
        }

        long long baseLL = static_cast<long long>(baseD);

        // Check overflow on addition with aug
        if ((aug > 0 && baseLL > std::numeric_limits<long long>::max() - aug) ||
            (aug < 0 && baseLL < std::numeric_limits<long long>::min() - aug)) {
            return -1;
        }

        long long next = baseLL + aug;
        if (next < 0) next = 0; // Clamp to non-negative population

        years++;
        if (next == cur) return -1; // Stagnation -> impossible to reach target
        cur = next;
    }

    return years;
}

int main() {
    // 5 test cases
    std::cout << "Test 1: " << nb_year(1000, 2.0, 50, 1200) << " (expected 3)\n";
    std::cout << "Test 2: " << nb_year(1500, 5.0, 100, 5000) << " (expected 15)\n";
    std::cout << "Test 3: " << nb_year(1500000, 2.5, 10000, 2000000) << " (expected 10)\n";
    std::cout << "Test 4: " << nb_year(1500, 0.0, 0, 1500) << " (expected 0)\n";
    std::cout << "Test 5: " << nb_year(1000, 0.0, 0, 2000) << " (expected -1)\n";
    return 0;
}