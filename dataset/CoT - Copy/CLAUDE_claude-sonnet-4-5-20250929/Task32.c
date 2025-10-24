
#include <stdio.h>

int calculateYears(double principal, double interest, double tax, double desired) {
    // Input validation
    if (principal <= 0 || desired <= 0 || interest < 0 || interest > 1 || tax < 0 || tax > 1) {
        fprintf(stderr, "Invalid input parameters\\n");
        return -1;
    }
    
    // Edge case: desired sum already reached
    if (desired <= principal) {
        return 0;
    }
    
    // Safety limit to prevent infinite loops
    const int MAX_YEARS = 1000;
    int years = 0;
    double currentPrincipal = principal;
    
    while (currentPrincipal < desired && years < MAX_YEARS) {
        double yearInterest = currentPrincipal * interest;
        double afterTax = yearInterest * (1 - tax);
        currentPrincipal += afterTax;
        years++;
    }
    
    return years;
}

int main() {
    // Test case 1: Example from problem
    printf("Test 1: %d years (Expected: 3)\\n", calculateYears(1000, 0.05, 0.18, 1100));
    
    // Test case 2: Already at desired sum
    printf("Test 2: %d years (Expected: 0)\\n", calculateYears(1000, 0.05, 0.18, 1000));
    
    // Test case 3: Higher interest rate
    printf("Test 3: %d years (Expected: 3)\\n", calculateYears(1000, 0.10, 0.18, 1200));
    
    // Test case 4: Lower tax rate
    printf("Test 4: %d years (Expected: 2)\\n", calculateYears(1000, 0.05, 0.10, 1100));
    
    // Test case 5: Larger gap
    printf("Test 5: %d years (Expected: 17)\\n", calculateYears(1000, 0.05, 0.18, 2000));
    
    return 0;
}
