
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// Function to calculate years needed for investment to reach desired sum
// Returns -1 on error, otherwise returns number of years
int calculateYears(double principal, double interest, double tax, double desired) {
    // Input validation: check for NaN and infinity to prevent undefined behavior
    if (isnan(principal) || isinf(principal) ||
        isnan(interest) || isinf(interest) ||
        isnan(tax) || isinf(tax) ||
        isnan(desired) || isinf(desired)) {
        fprintf(stderr, "Error: Invalid input - NaN or infinity not allowed\\n");
        return -1;
    }
    
    // Validate principal is positive to prevent invalid calculations
    if (principal <= 0.0) {
        fprintf(stderr, "Error: Principal must be positive\\n");
        return -1;
    }
    
    // Validate desired is positive
    if (desired <= 0.0) {
        fprintf(stderr, "Error: Desired sum must be positive\\n");
        return -1;
    }
    
    // Validate interest rate is non-negative and reasonable (0 to 100%)
    if (interest < 0.0 || interest > 1.0) {
        fprintf(stderr, "Error: Interest rate must be between 0 and 1\\n");
        return -1;
    }
    
    // Validate tax rate is non-negative and reasonable (0 to 100%)
    if (tax < 0.0 || tax > 1.0) {
        fprintf(stderr, "Error: Tax rate must be between 0 and 1\\n");
        return -1;
    }
    
    // Handle edge case: desired equals or less than principal
    if (desired <= principal) {
        return 0;
    }
    
    // Calculate net interest rate after tax to prevent recalculation in loop
    double netInterestRate = interest * (1.0 - tax);
    
    // Validate that investment can grow (net interest must be positive)
    if (netInterestRate <= 0.0) {
        fprintf(stderr, "Error: Net interest rate must be positive for growth\\n");
        return -1;
    }
    
    // Initialize current principal for iteration
    double currentPrincipal = principal;
    int years = 0;
    
    // Maximum iterations to prevent infinite loop in case of numerical issues
    const int MAX_YEARS = 1000000;
    
    // Calculate years needed, checking against maximum to prevent DoS
    while (currentPrincipal < desired && years < MAX_YEARS) {
        // Calculate interest earned this year
        double yearInterest = currentPrincipal * interest;
        
        // Calculate tax on interest (not on principal)
        double taxAmount = yearInterest * tax;
        
        // Add net interest to principal
        currentPrincipal += (yearInterest - taxAmount);
        
        years++;
        
        // Additional safety check for overflow or invalid state
        if (isnan(currentPrincipal) || isinf(currentPrincipal)) {
            fprintf(stderr, "Error: Calculation resulted in invalid value\\n");
            return -1;
        }
    }
    
    // Check if maximum iterations reached without convergence
    if (years >= MAX_YEARS) {
        fprintf(stderr, "Error: Maximum iterations reached - unable to reach desired sum\\n");
        return -1;
    }
    
    return years;
}

int main(void) {
    int result;
    
    // Test case 1: Example from problem description
    result = calculateYears(1000.0, 0.05, 0.18, 1100.0);
    if (result >= 0) {
        printf("Test 1: %d\\n", result);
    }
    
    // Test case 2: Edge case where desired equals principal
    result = calculateYears(1000.0, 0.05, 0.18, 1000.0);
    if (result >= 0) {
        printf("Test 2: %d\\n", result);
    }
    
    // Test case 3: Different values
    result = calculateYears(1500.0, 0.07, 0.15, 2000.0);
    if (result >= 0) {
        printf("Test 3: %d\\n", result);
    }
    
    // Test case 4: Low interest rate
    result = calculateYears(1000.0, 0.01, 0.10, 1050.0);
    if (result >= 0) {
        printf("Test 4: %d\\n", result);
    }
    
    // Test case 5: Higher tax rate
    result = calculateYears(5000.0, 0.08, 0.25, 6000.0);
    if (result >= 0) {
        printf("Test 5: %d\\n", result);
    }
    
    return 0;
}
