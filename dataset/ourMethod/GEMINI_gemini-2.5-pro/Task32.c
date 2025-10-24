#include <stdio.h>

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
    printf("Test Case 1:\n");
    printf("P=%.2f, I=%.2f, T=%.2f, D=%.2f -> Years: %d\n\n", p1, i1, t1, d1, years1);

    // Test Case 2: Desired sum is equal to principal
    double p2 = 1000.00, i2 = 0.05, t2 = 0.18, d2 = 1000.00;
    int years2 = calculateYears(p2, i2, t2, d2);
    printf("Test Case 2:\n");
    printf("P=%.2f, I=%.2f, T=%.2f, D=%.2f -> Years: %d\n\n", p2, i2, t2, d2, years2);

    // Test Case 3: Desired sum is less than principal
    double p3 = 1200.00, i3 = 0.05, t3 = 0.18, d3 = 1100.00;
    int years3 = calculateYears(p3, i3, t3, d3);
    printf("Test Case 3:\n");
    printf("P=%.2f, I=%.2f, T=%.2f, D=%.2f -> Years: %d\n\n", p3, i3, t3, d3, years3);

    // Test Case 4: No tax
    double p4 = 1000.00, i4 = 0.05, t4 = 0.0, d4 = 1100.00;
    int years4 = calculateYears(p4, i4, t4, d4);
    printf("Test Case 4:\n");
    printf("P=%.2f, I=%.2f, T=%.2f, D=%.2f -> Years: %d\n\n", p4, i4, t4, d4, years4);

    // Test Case 5: High tax and higher desired sum
    double p5 = 1000.00, i5 = 0.10, t5 = 0.50, d5 = 1200.00;
    int years5 = calculateYears(p5, i5, t5, d5);
    printf("Test Case 5:\n");
    printf("P=%.2f, I=%.2f, T=%.2f, D=%.2f -> Years: %d\n", p5, i5, t5, d5, years5);

    return 0;
}