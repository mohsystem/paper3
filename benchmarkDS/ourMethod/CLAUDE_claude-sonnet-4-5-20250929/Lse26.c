
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Error codes for secure error handling without exposing internals
#define SUCCESS 0
#define ERROR_INVALID_MONTH -1
#define ERROR_OVERFLOW -2

// Simulates retrieving monthly sales data
// In production, this would connect to a database or API with proper validation
int getMonthlySales(int month, double* result) {
    // Input validation: check for NULL pointer
    if (result == NULL) {
        return ERROR_INVALID_MONTH;
    }
    
    // Initialize output to safe value
    *result = 0.0;
    
    // Input validation: ensure month is within expected range
    if (month < 0 || month > 2) {
        // Fail closed with error code for invalid month index
        return ERROR_INVALID_MONTH;
    }
    
    // Simulated sales data for demonstration
    // In production, ensure data source is authenticated and validated
    const double salesData[3] = {15000.50, 18500.75, 22300.25};
    
    // Additional bounds check (defensive programming)
    if ((size_t)month >= sizeof(salesData) / sizeof(salesData[0])) {
        return ERROR_INVALID_MONTH;
    }
    
    *result = salesData[month];
    return SUCCESS;
}

int calculateQuarterlySales(double* result) {
    // Input validation: check for NULL pointer
    if (result == NULL) {
        return ERROR_INVALID_MONTH;
    }
    
    // Initialize output to safe value
    *result = 0.0;
    double sum = 0.0;
    
    // Iterate exactly 3 times for first quarter (months 0, 1, 2)
    for (int i = 0; i < 3; i++) {
        double monthlySale = 0.0;
        
        // Each iteration calls getMonthlySales with validated input
        int status = getMonthlySales(i, &monthlySale);
        if (status != SUCCESS) {
            return status;
        }
        
        // Check for overflow before addition (defensive programming)
        if (isinf(sum + monthlySale)) {
            return ERROR_OVERFLOW;
        }
        
        sum += monthlySale;
    }
    
    *result = sum;
    return SUCCESS;
}

int main(void) {
    int status = 0;
    double result = 0.0;
    
    // Test case 1: Normal quarterly calculation
    status = calculateQuarterlySales(&result);
    if (status == SUCCESS) {
        printf("Test 1 - Quarterly Sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 1 failed with error code: %d\\n", status);
    }
    
    // Test case 2: Verify individual month retrieval
    result = 0.0;
    status = getMonthlySales(0, &result);
    if (status == SUCCESS) {
        printf("Test 2 - Month 0 Sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 2 failed with error code: %d\\n", status);
    }
    
    // Test case 3: Verify boundary condition (month 2)
    result = 0.0;
    status = getMonthlySales(2, &result);
    if (status == SUCCESS) {
        printf("Test 3 - Month 2 Sales: %.2f\\n", result);
    } else {
        fprintf(stderr, "Test 3 failed with error code: %d\\n", status);
    }
    
    // Test case 4: Invalid month index (should fail)
    result = 0.0;
    status = getMonthlySales(3, &result);
    if (status == SUCCESS) {
        printf("Test 4 - Should not reach here: %.2f\\n", result);
    } else {
        printf("Test 4 - Expected error caught with code: %d\\n", status);
    }
    
    // Test case 5: Invalid negative month index (should fail)
    result = 0.0;
    status = getMonthlySales(-1, &result);
    if (status == SUCCESS) {
        printf("Test 5 - Should not reach here: %.2f\\n", result);
    } else {
        printf("Test 5 - Expected error caught with code: %d\\n", status);
    }
    
    return EXIT_SUCCESS;
}
