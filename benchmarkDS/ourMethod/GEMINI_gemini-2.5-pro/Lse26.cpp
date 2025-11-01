#include <iostream>
#include <vector>
#include <numeric>
#include <iomanip>

/**
 * @brief Simulates fetching monthly sales data.
 *
 * @param month The month index (0 for January, 1 for February, etc.).
 * @param monthlySalesData A vector containing sales data for each month.
 * @return The sales for the given month. Returns 0.0 if the month is out of bounds.
 */
double getMonthlySales(int month, const std::vector<double>& monthlySalesData) {
    // Input validation: ensure the month index is within the bounds of the data vector.
    // Using .at() would throw an exception, which is also a valid strategy.
    // Here we manually check and return 0.0 to fail safely.
    if (month >= 0 && static_cast<size_t>(month) < monthlySalesData.size()) {
        return monthlySalesData[static_cast<size_t>(month)];
    }
    return 0.0;
}

/**
 * @brief Calculates the total sales for the first quarter (first 3 months).
 *
 * @param salesData A vector containing at least 3 months of sales data.
 * @return The total sales for the first quarter.
 */
double calculateFirstQuarterSales(const std::vector<double>& salesData) {
    // Input validation: ensure salesData has enough data for a quarter.
    if (salesData.size() < 3) {
        return 0.0; // Fail safe by returning 0
    }

    double sum = 0.0;
    // The for loop iterates 3 times for the first quarter (months 0, 1, 2).
    for (int i = 0; i < 3; ++i) {
        // On each iteration, the loop calls getMonthlySales with the current value of i.
        // The return value is added to sum.
        sum += getMonthlySales(i, salesData);
    }
    // After the loop, sum contains the total sales for the first quarter.
    return sum;
}

int main() {
    // 5 Test Cases
    std::cout << "--- 5 Test Cases ---" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    // Test Case 1: Standard sales data
    std::vector<double> sales1 = {1500.50, 2200.00, 1850.75, 3000.00};
    std::cout << "Test Case 1 Total: " << calculateFirstQuarterSales(sales1) << std::endl; // Expected: 5551.25

    // Test Case 2: Sales data with zeros
    std::vector<double> sales2 = {1000.00, 0.00, 500.00};
    std::cout << "Test Case 2 Total: " << calculateFirstQuarterSales(sales2) << std::endl; // Expected: 1500.00

    // Test Case 3: High value sales data
    std::vector<double> sales3 = {100000.00, 150000.50, 125000.25, 90000.00};
    std::cout << "Test Case 3 Total: " << calculateFirstQuarterSales(sales3) << std::endl; // Expected: 375000.75

    // Test Case 4: Insufficient data (less than 3 months)
    std::vector<double> sales4 = {500.00, 600.00};
    std::cout << "Test Case 4 Total: " << calculateFirstQuarterSales(sales4) << std::endl; // Expected: 0.00

    // Test Case 5: Empty data
    std::vector<double> sales5 = {};
    std::cout << "Test Case 5 Total: " << calculateFirstQuarterSales(sales5) << std::endl; // Expected: 0.00

    return 0;
}