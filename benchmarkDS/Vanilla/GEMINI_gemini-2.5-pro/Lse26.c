#include <stdio.h>
#include <stddef.h>

/**
 * @brief A mock function to get sales for a given month (0-indexed) from a data source.
 * 
 * @param month The month index (0 for Jan, 1 for Feb, etc.).
 * @param salesData An array containing monthly sales data.
 * @param size The size of the salesData array.
 * @return The sales amount for the given month.
 */
double getMonthlySales(int month, const double salesData[], size_t size) {
    if (month >= 0 && month < size) {
        return salesData[month];
    }
    return 0.0;
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 * 
 * @param salesData An array containing at least 3 months of sales data.
 * @param size The size of the salesData array.
 * @return The total sales for the first quarter.
 */
double calculateFirstQuarterSales(const double salesData[], size_t size) {
    double sum = 0.0;
    // The for loop iterates 3 times starting at 0 and ending at 2.
    for (int i = 0; i < 3; ++i) {
        // On each iteration the loop calls getMonthlySales with the current value of i.
        double monthlySale = getMonthlySales(i, salesData, size);
        // The return value of getMonthlySales is added to sum.
        sum += monthlySale;
    }
    // After the loop; sum contains the total sales for the first quarter.
    return sum;
}

int main() {
    printf("--- C: 5 Test Cases ---\n");

    // Test Case 1
    double sales1[] = {1000.50, 1200.75, 1100.25, 1300.00};
    size_t size1 = sizeof(sales1) / sizeof(sales1[0]);
    double total1 = calculateFirstQuarterSales(sales1, size1);
    printf("Test Case 1: First quarter sales: %.2f\n", total1);

    // Test Case 2
    double sales2[] = {2500.00, 2600.00, 2700.00, 2800.00};
    size_t size2 = sizeof(sales2) / sizeof(sales2[0]);
    double total2 = calculateFirstQuarterSales(sales2, size2);
    printf("Test Case 2: First quarter sales: %.2f\n", total2);

    // Test Case 3
    double sales3[] = {99.99, 100.01, 250.50, 300.00};
    size_t size3 = sizeof(sales3) / sizeof(sales3[0]);
    double total3 = calculateFirstQuarterSales(sales3, size3);
    printf("Test Case 3: First quarter sales: %.2f\n", total3);

    // Test Case 4: All zero sales
    double sales4[] = {0.0, 0.0, 0.0, 100.0};
    size_t size4 = sizeof(sales4) / sizeof(sales4[0]);
    double total4 = calculateFirstQuarterSales(sales4, size4);
    printf("Test Case 4: First quarter sales: %.2f\n", total4);

    // Test Case 5: Large numbers
    double sales5[] = {1500000.78, 2300000.45, 1950000.12, 2100000.00};
    size_t size5 = sizeof(sales5) / sizeof(sales5[0]);
    double total5 = calculateFirstQuarterSales(sales5, size5);
    printf("Test Case 5: First quarter sales: %.2f\n", total5);

    return 0;
}