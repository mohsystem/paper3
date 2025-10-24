#include <stdio.h>

/**
 * @brief Calculates the number of years required for a principal to reach a desired sum.
 * 
 * @param principal The initial principal amount.
 * @param interest The annual interest rate.
 * @param tax The tax rate on the interest earned.
 * @param desired The desired final sum.
 * @return The number of years as an integer.
 */
int calculateYears(double principal, double interest, double tax, double desired) {
    if (desired <= principal) {
        return 0;
    }

    int years = 0;
    double currentPrincipal = principal;
    
    // The effective interest rate after tax
    double effectiveInterest = interest * (1.0 - tax);

    while (currentPrincipal < desired) {
        currentPrincipal += currentPrincipal * effectiveInterest;
        years++;
    }

    return years;
}

int main() {
    // Test Case 1: Example from description
    printf("Test Case 1: %d\n", calculateYears(1000.00, 0.05, 0.18, 1100.00));
    
    // Test Case 2: Longer period
    printf("Test Case 2: %d\n", calculateYears(1000.00, 0.01625, 0.18, 1200.00));

    // Test Case 3: Desired amount is same as principal
    printf("Test Case 3: %d\n", calculateYears(1000.00, 0.05, 0.18, 1000.00));

    // Test Case 4: Different values
    printf("Test Case 4: %d\n", calculateYears(1200.00, 0.05, 0.10, 1300.00));

    // Test Case 5: Higher tax rate
    printf("Test Case 5: %d\n", calculateYears(1500.00, 0.07, 0.60, 2000.00));

    return 0;
}