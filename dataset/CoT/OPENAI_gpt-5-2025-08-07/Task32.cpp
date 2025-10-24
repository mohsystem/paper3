// Chain-of-Through process:
// 1) Problem understanding:
//    - Compute years for P to reach D with yearly interest I and tax T on interest.
// 2) Security requirements:
//    - Validate inputs; avoid infinite loops when growth is non-positive.
// 3) Secure coding generation:
//    - Use finite checks, non-negative validation, and effective rate guard.
// 4) Code review:
//    - Confirmed correct math and loop termination.
// 5) Secure code output:
//    - Returns -1 for invalid/unreachable, otherwise years.

#include <iostream>
#include <cmath>
#include <limits>

int calculateYears(double principal, double interest, double tax, double desired) {
    auto isFinite = [](double x) {
        return std::isfinite(x);
    };
    if (!isFinite(principal) || !isFinite(interest) || !isFinite(tax) || !isFinite(desired)) {
        return -1;
    }
    if (principal < 0.0 || interest < 0.0 || tax < 0.0 || desired < 0.0) {
        return -1;
    }
    if (desired <= principal) {
        return 0;
    }

    double effectiveRate = interest * (1.0 - tax);
    if (effectiveRate <= 0.0) {
        return -1;
    }

    int years = 0;
    double p = principal;
    const int MAX_YEARS = 100000000;
    while (p < desired) {
        double interestEarned = p * interest;
        p += interestEarned * (1.0 - tax);
        years++;
        if (years > MAX_YEARS) {
            return -1;
        }
    }
    return years;
}

int main() {
    std::cout << calculateYears(1000.0, 0.05, 0.18, 1100.0) << "\n"; // Expected: 3
    std::cout << calculateYears(1000.0, 0.05, 0.18, 1000.0) << "\n"; // Expected: 0
    std::cout << calculateYears(1000.0, 0.00, 0.10, 1200.0) << "\n"; // Expected: -1 (unreachable)
    std::cout << calculateYears(1500.0, 0.07, 0.20, 2000.0) << "\n"; // Expected: 6
    std::cout << calculateYears(500.0, 0.05, 0.00, 525.0) << "\n";   // Expected: 1
    return 0;
}