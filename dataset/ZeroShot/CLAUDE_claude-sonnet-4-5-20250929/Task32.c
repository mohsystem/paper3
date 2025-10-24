
#include <stdio.h>

int calculateYears(double principal, double interest, double tax, double desired) {
    if (desired <= principal) {
        return 0;
    }
    
    int years = 0;
    double currentAmount = principal;
    
    while (currentAmount < desired) {
        double yearInterest = currentAmount * interest;
        double taxAmount = yearInterest * tax;
        double netInterest = yearInterest - taxAmount;
        currentAmount += netInterest;
        years++;
    }
    
    return years;
}

int main() {
    // Test case 1: Example from prompt
    printf("Test 1: %d\\n", calculateYears(1000, 0.05, 0.18, 1100));
    
    // Test case 2: Desired equals principal
    printf("Test 2: %d\\n", calculateYears(1000, 0.05, 0.18, 1000));
    
    // Test case 3: Higher interest rate
    printf("Test 3: %d\\n", calculateYears(1000, 0.10, 0.18, 1200));
    
    // Test case 4: Lower tax rate
    printf("Test 4: %d\\n", calculateYears(1500, 0.06, 0.10, 2000));
    
    // Test case 5: Large desired amount
    printf("Test 5: %d\\n", calculateYears(5000, 0.07, 0.15, 10000));
    
    return 0;
}
