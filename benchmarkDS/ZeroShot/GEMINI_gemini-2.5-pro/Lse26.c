#include <stdio.h>
#include <stddef.h> // For size_t
#include <stdlib.h> // For NULL

// Using long long for sales data to prevent integer overflow.
typedef long long sales_type;

/**
 * @brief Retrieves the sales for a specific month from a data source.
 * Secure: Performs bounds checking to prevent accessing out-of-bounds memory.
 * @param monthIndex The index of the month (0-2).
 * @param salesData An array containing monthly sales data.
 * @param dataSize The number of elements in the salesData array.
 * @return The sales for the given month, or 0 if the index is invalid.
 */
sales_type getMonthlySales(int monthIndex, const sales_type salesData[], size_t dataSize) {
    if (salesData == NULL || monthIndex < 0 || (size_t)monthIndex >= dataSize) {
        // Handle invalid input gracefully.
        return 0;
    }
    return salesData[monthIndex];
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 * The for loop iterates 3 times. On each iteration, it calls getMonthlySales
 * with the current loop index i. The return value is added to a running sum.
 * @param firstQuarterSalesData An array with at least 3 elements representing sales.
 * @param dataSize The number of elements in the firstQuarterSalesData array.
 * @return The sum of the first three months' sales.
 */
sales_type calculateFirstQuarterSales(const sales_type firstQuarterSalesData[], size_t dataSize) {
    // Use long long for sum to prevent potential integer overflow.
    sales_type sum = 0;
    
    // The loop iterates 3 times for the first quarter (months 0, 1, 2).
    for (int i = 0; i < 3; ++i) {
        // On each iteration, get the sales for the month and add to sum.
        sum += getMonthlySales(i, firstQuarterSalesData, dataSize);
    }
    
    return sum;
}

int main() {
    // 5 test cases to validate the functionality.
    sales_type testCase1[] = {10000LL, 12000LL, 15000LL};
    sales_type testCase2[] = {0LL, 0LL, 0LL};
    sales_type testCase3[] = {9000000000000000000LL, 10LL, 20LL};
    sales_type testCase4[] = {1500LL, 2500LL, 500LL};
    sales_type testCase5[] = {-100LL, 500LL, -200LL};

    // Array of pointers to the test case arrays
    sales_type* testCases[] = {testCase1, testCase2, testCase3, testCase4, testCase5};
    size_t num_test_cases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running 5 C test cases...\n");
    for (size_t i = 0; i < num_test_cases; ++i) {
        sales_type* currentCase = testCases[i];
        // For this example, all test arrays have 3 elements.
        size_t caseSize = 3;
        sales_type totalSales = calculateFirstQuarterSales(currentCase, caseSize);
        printf("Test Case %zu: [%lld, %lld, %lld] -> Total Sales: %lld\n",
               i + 1, currentCase[0], currentCase[1], currentCase[2], totalSales);
    }

    return 0;
}