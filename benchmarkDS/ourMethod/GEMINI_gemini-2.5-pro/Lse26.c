#include <stdio.h>
#include <stddef.h>

/**
 * @brief Simulates fetching monthly sales data.
 *
 * @param month The month index (0 for January, 1 for February, etc.).
 * @param monthlySalesData A pointer to an array containing sales data.
 * @param size The number of elements in the monthlySalesData array.
 * @return The sales for the given month. Returns 0.0 if the month is out of bounds.
 */
double getMonthlySales(int month, const double monthlySalesData[], size_t size) {
    // Input validation: ensure the data pointer is not NULL and the index is within bounds.
    if (monthlySalesData != NULL && month >= 0 && (size_t)month < size) {
        return monthlySalesData[month];
    }
    return 0.0; // Fail safe
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 *
 * @param salesData A pointer to an array with at least 3 months of sales data.
 * @param size The number of elements in the salesData array.
 * @return The total sales for the first quarter.
 */
double calculateFirstQuarterSales(const double salesData[], size_t size) {
    // Input validation: ensure data is valid and has enough entries for a quarter.
    if (salesData == NULL || size < 3) {
        return 0.0; // Fail safe
    }

    double sum = 0.0;
    // The for loop iterates 3 times for the first quarter (months 0, 1, 2).
    for (int i = 0; i < 3; ++i) {
        // On each iteration, the loop calls getMonthlySales with the current value of i.
        // The return value is added to sum.
        sum += getMonthlySales(i, salesData, size);
    }
    // After the loop, sum contains the total sales for the first quarter.
    return sum;
}

int main() {
    // 5 Test Cases
    printf("--- 5 Test Cases ---\n");

    // Test Case 1: Standard sales data
    double sales1[] = {1500.50, 2200.00, 1850.75, 3000.00};
    size_t size1 = sizeof(sales1) / sizeof(sales1[0]);
    printf("Test Case 1 Total: %.2f\n", calculateFirstQuarterSales(sales1, size1)); // Expected: 5551.25

    // Test Case 2: Sales data with zeros
    double sales2[] = {1000.00, 0.00, 500.00};
    size_t size2 = sizeof(sales2) / sizeof(sales2[0]);
    printf("Test Case 2 Total: %.2f\n", calculateFirstQuarterSales(sales2, size2)); // Expected: 1500.00

    // Test Case 3: High value sales data
    double sales3[] = {100000.00, 150000.50, 125000.25, 90000.00};
    size_t size3 = sizeof(sales3) / sizeof(sales3[0]);
    printf("Test Case 3 Total: %.2f\n", calculateFirstQuarterSales(sales3, size3)); // Expected: 375000.75

    // Test Case 4: Insufficient data (less than 3 months)
    double sales4[] = {500.00, 600.00};
    size_t size4 = sizeof(sales4) / sizeof(sales4[0]);
    printf("Test Case 4 Total: %.2f\n", calculateFirstQuarterSales(sales4, size4)); // Expected: 0.00

    // Test Case 5: Null data
    double* sales5 = NULL;
    size_t size5 = 0;
    printf("Test Case 5 Total: %.2f\n", calculateFirstQuarterSales(sales5, size5)); // Expected: 0.00
    
    return 0;
}