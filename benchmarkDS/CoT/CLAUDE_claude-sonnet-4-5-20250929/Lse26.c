
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Simulated monthly sales data
static const double MONTHLY_SALES[] = {
    15000.0, 18000.0, 16500.0, 20000.0,
    17500.0, 19000.0, 21000.0, 18500.0,
    16000.0, 22000.0, 19500.0, 23000.0
};

#define MONTHLY_SALES_SIZE (sizeof(MONTHLY_SALES) / sizeof(MONTHLY_SALES[0]))

// Secure method to get monthly sales with input validation
bool getMonthlySales(int month, double* result) {
    // Validate month index to prevent out-of-bounds access
    if (month < 0 || month >= (int)MONTHLY_SALES_SIZE) {
        fprintf(stderr, "Error: Invalid month index: %d\\n", month);
        return false;
    }
    
    *result = MONTHLY_SALES[month];
    return true;
}

// Calculate first quarter sales
bool calculateFirstQuarterSales(double* sum) {
    if (sum == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to calculateFirstQuarterSales\\n");
        return false;
    }
    
    *sum = 0.0;
    
    // Iterate through first 3 months (index 0 to 2)
    for (int i = 0; i < 3; i++) {
        double monthlySale;
        if (!getMonthlySales(i, &monthlySale)) {
            return false;
        }
        
        // Check for valid sales value
        if (monthlySale < 0) {
            fprintf(stderr, "Error: Sales value cannot be negative\\n");
            return false;
        }
        
        *sum += monthlySale;
    }
    
    return true;
}

int main() {
    double quarterSales;
    
    printf("Test Case 1: First Quarter Sales\\n");
    if (calculateFirstQuarterSales(&quarterSales)) {
        printf("Total: $%.2f\\n", quarterSales);
    }
    
    printf("\\nTest Case 2: Individual Monthly Sales\\n");
    for (int i = 0; i < 3; i++) {
        double sale;
        if (getMonthlySales(i, &sale)) {
            printf("Month %d: $%.2f\\n", i, sale);
        }
    }
    
    printf("\\nTest Case 3: Verify sum calculation\\n");
    double s0, s1, s2;
    if (getMonthlySales(0, &s0) && getMonthlySales(1, &s1) && getMonthlySales(2, &s2)) {
        double manual = s0 + s1 + s2;
        printf("Manual calculation: $%.2f\\n", manual);
    }
    
    printf("\\nTest Case 4: Valid month access\\n");
    double sale5;
    if (getMonthlySales(5, &sale5)) {
        printf("Month 5 sales: $%.2f\\n", sale5);
    }
    
    printf("\\nTest Case 5: Error handling\\n");
    double invalid;
    if (!getMonthlySales(15, &invalid)) {
        printf("Successfully handled invalid month access\\n");
    }
    
    return 0;
}
