
#include <stdio.h>

double getMonthlySales(int month) {
    // Simulated monthly sales data (in a real application, this might come from a database)
    static const double salesData[] = {
        15000.50, 22000.75, 18500.25, 25000.00, 30000.50, 
        27000.75, 32000.25, 28000.50, 24000.75, 26000.25, 
        31000.50, 35000.75
    };
    static const int dataSize = sizeof(salesData) / sizeof(salesData[0]);
    
    // Validate month index
    if (month >= 0 && month < dataSize) {
        return salesData[month];
    }
    return 0.0;
}

double calculateFirstQuarterSales() {
    double sum = 0.0;
    
    for (int i = 0; i < 3; i++) {
        sum += getMonthlySales(i);
    }
    
    return sum;
}

int main() {
    // Test case 1: Calculate first quarter sales
    printf("Test Case 1 - First Quarter Sales: $%.2f\\n", calculateFirstQuarterSales());
    
    // Test case 2: Verify individual months
    printf("\\nTest Case 2 - Individual Month Sales:\\n");
    for (int i = 0; i < 3; i++) {
        printf("Month %d: $%.2f\\n", i, getMonthlySales(i));
    }
    
    // Test case 3: Manual calculation verification
    printf("\\nTest Case 3 - Manual Verification:\\n");
    double manual = getMonthlySales(0) + getMonthlySales(1) + getMonthlySales(2);
    printf("Manual calculation: $%.2f\\n", manual);
    
    // Test case 4: Check with different starting data
    printf("\\nTest Case 4 - Second Quarter Sales:\\n");
    double q2Sum = 0.0;
    for (int i = 3; i < 6; i++) {
        q2Sum += getMonthlySales(i);
    }
    printf("Second Quarter Sales: $%.2f\\n", q2Sum);
    
    // Test case 5: Validate boundary conditions
    printf("\\nTest Case 5 - Boundary Conditions:\\n");
    printf("Invalid month (-1): $%.2f\\n", getMonthlySales(-1));
    printf("Invalid month (12): $%.2f\\n", getMonthlySales(12));
    printf("Valid month (0): $%.2f\\n", getMonthlySales(0));
    
    return 0;
}
