
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

// Function to calculate total sales from three monthly sales values
// Returns true on success, false on error
// Result is stored in the output parameter
bool calculateTotalSales(double month1, double month2, double month3, double* result) {
    // Validate output pointer is not NULL
    if (result == NULL) {
        fprintf(stderr, "Error: Output parameter is NULL\\n");
        return false;
    }
    
    // Initialize result to 0.0
    *result = 0.0;
    
    // Validate inputs are non-negative (sales cannot be negative)
    if (month1 < 0.0 || month2 < 0.0 || month3 < 0.0) {
        fprintf(stderr, "Error: Sales values cannot be negative\\n");
        return false;
    }
    
    // Check for NaN or infinity in inputs
    if (isnan(month1) || isnan(month2) || isnan(month3) ||
        isinf(month1) || isinf(month2) || isinf(month3)) {
        fprintf(stderr, "Error: Sales values must be valid finite numbers\\n");
        return false;
    }
    
    // Initialize totalSales to 0.0
    double totalSales = 0.0;
    
    // Add each month's sales to totalSales
    totalSales += month1;
    totalSales += month2;
    totalSales += month3;
    
    // Verify result is valid
    if (isnan(totalSales) || isinf(totalSales)) {
        fprintf(stderr, "Error: Total sales calculation resulted in invalid value\\n");
        return false;
    }
    
    // Store result in output parameter
    *result = totalSales;
    return true;
}

int main(void) {
    double result = 0.0;
    
    // Test case 1: Normal positive values
    if (calculateTotalSales(1000.50, 1500.75, 2000.25, &result)) {
        printf("Test 1 - Total sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 1 failed\\n");
    }
    
    // Test case 2: Zero values
    if (calculateTotalSales(0.0, 0.0, 0.0, &result)) {
        printf("Test 2 - Total sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 2 failed\\n");
    }
    
    // Test case 3: Large values
    if (calculateTotalSales(50000.99, 75000.50, 100000.01, &result)) {
        printf("Test 3 - Total sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 3 failed\\n");
    }
    
    // Test case 4: Negative value (should fail)
    if (calculateTotalSales(1000.0, -500.0, 2000.0, &result)) {
        printf("Test 4 - Total sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 4 failed (expected)\\n");
    }
    
    // Test case 5: Mixed decimal values
    if (calculateTotalSales(123.45, 678.90, 234.56, &result)) {
        printf("Test 5 - Total sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 5 failed\\n");
    }
    
    return 0;
}
