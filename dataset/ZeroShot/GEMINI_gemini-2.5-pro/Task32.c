#include <stdio.h>

/**
 * @brief Calculates the number of years to reach a desired sum.
 *
 * @param principal The initial principal amount.
 * @param interest  The annual interest rate (e.g., 0.05 for 5%).
 * @param tax       The tax rate on interest earned (e.g., 0.18 for 18%).
 * @param desired   The desired final sum.
 * @return The number of whole years required.
 */
int calculateYears(double principal, double interest, double tax, double desired) {
    if (principal >= desired) {
        return 0;
    }

    int years = 0;
    double currentPrincipal = principal;

    // Assuming interest is positive and tax is less than 1.0 to avoid infinite loops.
    while (currentPrincipal < desired) {
        double interestEarned = currentPrincipal * interest;
        double taxPaid = interestEarned * tax;
        currentPrincipal += (interestEarned - taxPaid);
        years++;
    }

    return years;
}

int main() {
    // Test Case 1: From description
    printf("Test Case 1: %d\n", calculateYears(1000.00, 0.05, 0.18, 1100.00)); // Expected: 3

    // Test Case 2: Desired amount is already met
    printf("Test Case 2: %d\n", calculateYears(1000.00, 0.05, 0.18, 1000.00)); // Expected: 0

    // Test Case 3: Larger growth needed
    printf("Test Case 3: %d\n", calculateYears(100.00, 0.10, 0.20, 200.00)); // Expected: 10

    // Test Case 4: No tax
    printf("Test Case 4: %d\n", calculateYears(100.00, 0.10, 0.0, 150.00)); // Expected: 5

    // Test Case 5: High tax
    printf("Test Case 5: %d\n", calculateYears(1000.00, 0.08, 0.50, 1100.00)); // Expected: 3

    return 0;
}