#include <stdio.h>
#include <stddef.h>

/**
 * @brief Simulates fetching monthly sales data for a given month index.
 * 
 * In C, it's crucial to pass the array's size to perform safe bounds checking.
 * @param salesData A pointer to an array of doubles.
 * @param size The number of elements in the salesData array.
 * @param monthIndex The index of the month (0-based).
 * @return The sales figure for the given month, or 0.0 if the index is out of bounds.
 */
double getMonthlySales(const double salesData[], size_t size, int monthIndex) {
    if (monthIndex < 0 || monthIndex >= size) {
        // Handle invalid index to prevent out-of-bounds memory access.
        return 0.0;
    }
    return salesData[monthIndex];
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 * 
 * It iterates three times, calling getMonthlySales and summing the results.
 * @param yearlySales A pointer to an array with at least 3 months of sales data.
 * @param size The number of elements in the yearlySales array.
 * @return The total sum of sales for the first quarter.
 */
double calculateFirstQuarterSales(const double yearlySales[], size_t size) {
    // Using double for sum prevents potential integer overflow with large sales numbers.
    double sum = 0.0;
    
    // The for loop iterates 3 times (for i = 0, 1, and 2) for the first quarter.
    for (int i = 0; i < 3; i++) {
        // On each iteration, the loop calls getMonthlySales with the current value of i.
        // The return value is added to sum.
        sum += getMonthlySales(yearlySales, size, i);
    }
    
    // After the loop, sum contains the total sales for the first quarter.
    return sum;
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1: Standard sales data
    double sales1[] = {10000.50, 15000.75, 12500.00, 18000.25};
    size_t size1 = sizeof(sales1) / sizeof(sales1[0]);
    printf("Test Case 1 - Expected: 37501.25, Actual: %.2f\n", calculateFirstQuarterSales(sales1, size1));

    // Test Case 2: Sales data with a zero-sale month
    double sales2[] = {22000.00, 0.00, 25000.50, 21000.00};
    size_t size2 = sizeof(sales2) / sizeof(sales2[0]);
    printf("Test Case 2 - Expected: 47000.50, Actual: %.2f\n", calculateFirstQuarterSales(sales2, size2));

    // Test Case 3: All sales figures are identical
    double sales3[] = {5000.00, 5000.00, 5000.00, 5000.00};
    size_t size3 = sizeof(sales3) / sizeof(sales3[0]);
    printf("Test Case 3 - Expected: 15000.00, Actual: %.2f\n", calculateFirstQuarterSales(sales3, size3));

    // Test Case 4: Larger sales figures to test data type limits
    double sales4[] = {1234567.89, 9876543.21, 5555555.55, 9012345.67};
    size_t size4 = sizeof(sales4) / sizeof(sales4[0]);
    printf("Test Case 4 - Expected: 16666666.65, Actual: %.2f\n", calculateFirstQuarterSales(sales4, size4));
    
    // Test Case 5: Input array with exactly 3 elements
    double sales5[] = {100.00, 200.00, 300.00};
    size_t size5 = sizeof(sales5) / sizeof(sales5[0]);
    printf("Test Case 5 - Expected: 600.00, Actual: %.2f\n", calculateFirstQuarterSales(sales5, size5));

    return 0;
}