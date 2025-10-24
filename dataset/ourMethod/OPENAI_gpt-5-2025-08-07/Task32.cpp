#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

int yearsToReach(double P, double I, double T, double D) {
    if (!std::isfinite(P) || !std::isfinite(I) || !std::isfinite(T) || !std::isfinite(D)) return -1;
    if (P < 0.0 || D < 0.0) return -1;
    if (D <= P) return 0;
    if (I < 0.0 || T < 0.0 || T > 1.0) return -1;

    double eff = I * (1.0 - T);
    if (eff <= 0.0) return -1;

    const int maxYears = 100000000; // safety guard
    int years = 0;
    while (P < D) {
        double delta = P * eff;
        if (!std::isfinite(delta)) return -1;
        P += delta;
        years++;
        if (years > maxYears) return -1;
    }
    return years;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);

    // Test case 1: Example from prompt
    {
        double P = 1000.0, I = 0.05, T = 0.18, D = 1100.0;
        int y = yearsToReach(P, I, T, D);
        std::cout << "Test 1 (expect 3): " << y << "\n";
    }

    // Test case 2: Desired equals principal -> 0 years
    {
        double P = 1000.0, I = 0.05, T = 0.18, D = 1000.0;
        int y = yearsToReach(P, I, T, D);
        std::cout << "Test 2 (expect 0): " << y << "\n";
    }

    // Test case 3: Zero interest -> impossible
    {
        double P = 1000.0, I = 0.0, T = 0.18, D = 1100.0;
        int y = yearsToReach(P, I, T, D);
        std::cout << "Test 3 (expect -1): " << y << "\n";
    }

    // Test case 4: Tax 100% -> no growth -> impossible
    {
        double P = 500.0, I = 0.10, T = 1.0, D = 600.0;
        int y = yearsToReach(P, I, T, D);
        std::cout << "Test 4 (expect -1): " << y << "\n";
    }

    // Test case 5: Normal small growth
    {
        double P = 100.0, I = 0.02, T = 0.20, D = 120.0; // eff = 0.016
        int y = yearsToReach(P, I, T, D); // Expect around 12
        std::cout << "Test 5 (expect 12): " << y << "\n";
    }

    return 0;
}