#include <stdio.h>

double calculateTotalSales(double month1Sales, double month2Sales, double month3Sales) {
    // The first line declares a variable called totalSales.
    double totalSales = 0.0;
    
    // The next three lines add the sales for each month to the totalSales variable.
    totalSales += month1Sales;
    totalSales += month2Sales;
    totalSales += month3Sales;
    
    return totalSales;
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1
    double total1 = calculateTotalSales(1200.50, 1550.75, 1300.00);
    // The last line prints the total sales to the screen.
    printf("Test Case 1: Total Sales = %.2f\n", total1);

    // Test Case 2
    double total2 = calculateTotalSales(0.0, 0.0, 0.0);
    printf("Test Case 2: Total Sales = %.2f\n", total2);

    // Test Case 3
    double total3 = calculateTotalSales(99.99, 100.01, 250.50);
    printf("Test Case 3: Total Sales = %.2f\n", total3);
    
    // Test Case 4
    double total4 = calculateTotalSales(1000000.10, 2500000.25, 1500000.15);
    printf("Test Case 4: Total Sales = %.2f\n", total4);

    // Test Case 5
    double total5 = calculateTotalSales(1.0, 2.0, 3.0);
    printf("Test Case 5: Total Sales = %.2f\n", total5);

    return 0;
}