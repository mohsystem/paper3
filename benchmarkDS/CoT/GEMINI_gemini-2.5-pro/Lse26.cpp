#include <iostream>
#include <vector>
#include <iomanip>

/**
 * @brief Simulates fetching monthly sales data for a given month index.
 * 
 * As a security measure, it checks if the monthIndex is within the valid bounds of the vector.
 * @param salesData A constant reference to a vector containing sales data.
 * @param monthIndex The index of the month (0-based).
 * @return The sales figure for the given month, or 0.0 if the index is out of bounds.
 */
double getMonthlySales(const std::vector<double>& salesData, int monthIndex) {
    if (monthIndex < 0 || monthIndex >= salesData.size()) {
        // Handle invalid index to prevent out-of-bounds access.
        return 0.0;
    }
    return salesData[monthIndex];
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 * 
 * It iterates three times, calling getMonthlySales and summing the results.
 * @param yearlySales A constant reference to a vector with at least 3 months of sales data.
 * @return The total sum of sales for the first quarter.
 */
double calculateFirstQuarterSales(const std::vector<double>& yearlySales) {
    // Using double for sum prevents potential integer overflow with large sales numbers.
    double sum = 0.0;

    // The for loop iterates 3 times (for i = 0, 1, and 2) for the first quarter.
    for (int i = 0; i < 3; ++i) {
        // On each iteration, the loop calls getMonthlySales with the current value of i.
        // The return value is added to sum.
        sum += getMonthlySales(yearlySales, i);
    }
    
    // After the loop, sum contains the total sales for the first quarter.
    return sum;
}

int main() {
    // --- 5 Test Cases ---
    std::cout << std::fixed << std::setprecision(2);

    // Test Case 1: Standard sales data
    std::vector<double> sales1 = {10000.50, 15000.75, 12500.00, 18000.25};
    std::cout << "Test Case 1 - Expected: 37501.25, Actual: " << calculateFirstQuarterSales(sales1) << std::endl;

    // Test Case 2: Sales data with a zero-sale month
    std::vector<double> sales2 = {22000.00, 0.00, 25000.50, 21000.00};
    std::cout << "Test Case 2 - Expected: 47000.50, Actual: " << calculateFirstQuarterSales(sales2) << std::endl;

    // Test Case 3: All sales figures are identical
    std::vector<double> sales3 = {5000.00, 5000.00, 5000.00, 5000.00};
    std::cout << "Test Case 3 - Expected: 15000.00, Actual: " << calculateFirstQuarterSales(sales3) << std::endl;

    // Test Case 4: Larger sales figures to test data type limits
    std::vector<double> sales4 = {1234567.89, 9876543.21, 5555555.55, 9012345.67};
    std::cout << "Test Case 4 - Expected: 16666666.65, Actual: " << calculateFirstQuarterSales(sales4) << std::endl;
    
    // Test Case 5: Input vector with exactly 3 elements
    std::vector<double> sales5 = {100.00, 200.00, 300.00};
    std::cout << "Test Case 5 - Expected: 600.00, Actual: " << calculateFirstQuarterSales(sales5) << std::endl;

    return 0;
}