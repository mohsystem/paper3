#include <iostream>

/**
 * Calculates the number of years to reach a desired sum with yearly interest and tax.
 *
 * @param p The initial principal amount.
 * @param i The yearly interest rate (e.g., 0.05 for 5%).
 * @param t The tax rate on the interest earned (e.g., 0.18 for 18%).
 * @param d The desired final sum.
 * @return The number of years required.
 */
int calculateYears(double p, double i, double t, double d) {
    if (p >= d) {
        return 0;
    }

    int years = 0;
    double currentPrincipal = p;

    while (currentPrincipal < d) {
        double interestEarned = currentPrincipal * i;
        double taxOnInterest = interestEarned * t;
        currentPrincipal += (interestEarned - taxOnInterest);
        years++;
    }

    return years;
}

int main() {
    // Test Case 1: Example from description
    std::cout << "Test Case 1: " << calculateYears(1000.00, 0.05, 0.18, 1100.00) << std::endl; // Expected: 3

    // Test Case 2: Principal already meets or exceeds desired amount
    std::cout << "Test Case 2: " << calculateYears(1000.00, 0.05, 0.18, 1000.00) << std::endl; // Expected: 0

    // Test Case 3: No tax
    std::cout << "Test Case 3: " << calculateYears(1000.00, 0.05, 0.0, 1100.00) << std::endl;  // Expected: 2

    // Test Case 4: Higher desired amount
    std::cout << "Test Case 4: " << calculateYears(1200.00, 0.05, 0.10, 2000.00) << std::endl; // Expected: 12

    // Test Case 5: Higher tax rate
    std::cout << "Test Case 5: " << calculateYears(1000.00, 0.05, 0.50, 1100.00) << std::endl; // Expected: 4

    return 0;
}