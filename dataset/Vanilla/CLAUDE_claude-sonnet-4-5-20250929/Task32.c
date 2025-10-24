
#include <stdio.h>

int calculateYears(double principal, double interest, double tax, double desired) {
    if (principal >= desired) {
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
    // Test case 1: Example from problem
    printf("Test 1: %d (Expected: 3)\\n", calculateYears(1000.00, 0.05, 0.18, 1100.00));
    
    // Test case 2: Principal equals desired
    printf("Test 2: %d (Expected: 0)\\n", calculateYears(1000.00, 0.05, 0.18, 1000.00));
    
    // Test case 3: Small growth needed
    printf("Test 3: %d (Expected: 1)\\n", calculateYears(1000.00, 0.10, 0.10, 1050.00));
    
    // Test case 4: Large growth needed
    printf("Test 4: %d (Expected: 25)\\n", calculateYears(1000.00, 0.01, 0.18, 1200.00));
    
    // Test case 5: High interest, low tax
    printf("Test 5: %d (Expected: 4)\\n", calculateYears(1000.00, 0.20, 0.10, 2000.00));
    
    return 0;
}
