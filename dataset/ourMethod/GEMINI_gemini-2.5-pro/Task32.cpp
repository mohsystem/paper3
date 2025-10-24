#include <iostream>

/**
 * @brief Calculates the number of years to reach a desired sum.
 *
 * @param principal The initial principal amount.
 * @param interestRate The annual interest rate (e.g., 0.05 for 5%).
 * @param taxRate The tax rate on interest (e.g., 0.18 for 18%).
 * @param desiredSum The target amount of money.
 * @return The number of years required as an integer.
 */
int calculateYears(double principal, double interestRate, double taxRate, double desiredSum) {
    if (principal >= desiredSum) {
        return 0;
    }

    int years = 0;
    double currentPrincipal = principal;
    
    // Pre-calculate the effective interest rate after tax for efficiency.
    double effectiveInterestRate = interestRate * (1.0 - taxRate);

    // According to the problem's assumptions, principal will grow,
    // so we don't explicitly handle the case where effectiveInterestRate <= 0,
    // which would otherwise cause an infinite loop if desiredSum > principal.
    while (currentPrincipal < desiredSum) {
        currentPrincipal += currentPrincipal * effectiveInterestRate;
        years++;
    }

    return years;
}

int main() {
    // Test Case 1: Example from prompt
    double p1 = 1000.00, i1 = 0.05, t1 = 0.18, d1 = 1100.00;
    int years1 = calculateYears(p1, i1, t1, d1);
    std::cout << "Test Case 1:\n";
    std::cout << "P=" << p1 << ", I=" << i1 << ", T=" << t1 << ", D=" << d1 << " -> Years: " << years1 << "\n\n";

    // Test Case 2: Desired sum is equal to principal
    double p2 = 1000.00, i2 = 0.05, t2 = 0.18, d2 = 1000.00;
    int years2 = calculateYears(p2, i2, t2, d2);
    std::cout << "Test Case 2:\n";
    std::cout << "P=" << p2 << ", I=" << i2 << ", T=" << t2 << ", D=" << d2 << " -> Years: " << years2 << "\n\n";

    // Test Case 3: Desired sum is less than principal
    double p3 = 1200.00, i3 = 0.05, t3 = 0.18, d3 = 1100.00;
    int years3 = calculateYears(p3, i3, t3, d3);
    std::cout << "Test Case 3:\n";
    std::cout << "P=" << p3 << ", I=" << i3 << ", T=" << t3 << ", D=" << d3 << " -> Years: " << years3 << "\n\n";

    // Test Case 4: No tax
    double p4 = 1000.00, i4 = 0.05, t4 = 0.0, d4 = 1100.00;
    int years4 = calculateYears(p4, i4, t4, d4);
    std::cout << "Test Case 4:\n";
    std::cout << "P=" << p4 << ", I=" << i4 << ", T=" << t4 << ", D=" << d4 << " -> Years: " << years4 << "\n\n";

    // Test Case 5: High tax and higher desired sum
    double p5 = 1000.00, i5 = 0.10, t5 = 0.50, d5 = 1200.00;
    int years5 = calculateYears(p5, i5, t5, d5);
    std::cout << "Test Case 5:\n";
    std::cout << "P=" << p5 << ", I=" << i5 << ", T=" << t5 << ", D=" << d5 << " -> Years: " << years5 << "\n";

    return 0;
}