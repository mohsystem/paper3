#include <stdio.h>

double sumSales(double month1, double month2, double month3) {
    return month1 + month2 + month3;
}

int main(void) {
    // Test Case 1: Follow the exact prompt steps
    double totalSales = 0.0; // First line declares totalSales
    totalSales += 1200.50;   // Add month 1
    totalSales += 1340.75;   // Add month 2
    totalSales += 980.25;    // Add month 3
    printf("%.10f\n", totalSales); // Print total

    // Test Case 2
    totalSales = 0.0;
    totalSales = sumSales(0.0, 0.0, 0.0);
    printf("%.10f\n", totalSales);

    // Test Case 3
    totalSales = 0.0;
    totalSales = sumSales(500.0, 600.0, 700.0);
    printf("%.10f\n", totalSales);

    // Test Case 4: Large numbers
    totalSales = 0.0;
    totalSales = sumSales(1500000000.0, 1500000000.0, 1500000000.0);
    printf("%.10f\n", totalSales);

    // Test Case 5: Small decimals
    totalSales = 0.0;
    totalSales = sumSales(1e-9, 2e-9, 3e-9);
    printf("%.10f\n", totalSales);

    return 0;
}