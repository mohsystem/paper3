#include <stdio.h>

/**
 * @brief Calculates the total sales from three monthly sales figures.
 * @param month1Sales Sales for the first month.
 * @param month2Sales Sales for the second month.
 * @param month3Sales Sales for the third month.
 * @return The total sales.
 */
double calculateTotalSales(double month1Sales, double month2Sales, double month3Sales) {
    // The first line declares a variable called totalSales.
    double totalSales = 0.0;
    
    // The next three lines add the sales for each month to the totalSales variable.
    totalSales += month1Sales;
    totalSales += month2Sales;
    totalSales += month3Sales;
    
    // The function returns the total sales.
    return totalSales;
}

int main() {
    // 5 test cases
    
    // Test Case 1
    double total1 = calculateTotalSales(1500.50, 2200.75, 1850.25);
    printf("Test Case 1 Total Sales: %.2f\n", total1);

    // Test Case 2
    double total2 = calculateTotalSales(100.0, 200.0, 300.0);
    printf("Test Case 2 Total Sales: %.2f\n", total2);

    // Test Case 3 (with zeros)
    double total3 = calculateTotalSales(0.0, 550.50, 0.0);
    printf("Test Case 3 Total Sales: %.2f\n", total3);

    // Test Case 4 (large numbers)
    double total4 = calculateTotalSales(123456.78, 987654.32, 555555.55);
    printf("Test Case 4 Total Sales: %.2f\n", total4);

    // Test Case 5 (all same values)
    double total5 = calculateTotalSales(777.77, 777.77, 777.77);
    printf("Test Case 5 Total Sales: %.2f\n", total5);

    return 0;
}