#include <iostream>
#include <vector>
#include <iomanip>

/**
 * @brief A mock function to get sales for a given month (0-indexed) from a data source.
 * 
 * @param month The month index (0 for Jan, 1 for Feb, etc.).
 * @param salesData A vector containing monthly sales data.
 * @return The sales amount for the given month.
 */
double getMonthlySales(int month, const std::vector<double>& salesData) {
    if (month >= 0 && month < salesData.size()) {
        return salesData[month];
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
    double sum = 0.0;
    // The for loop iterates 3 times starting at 0 and ending at 2.
    for (int i = 0; i < 3; ++i) {
        // On each iteration the loop calls getMonthlySales with the current value of i.
        double monthlySale = getMonthlySales(i, salesData);
        // The return value of getMonthlySales is added to sum.
        sum += monthlySale;
    }
    // After the loop; sum contains the total sales for the first quarter.
    return sum;
}

int main() {
    std::cout << "--- C++: 5 Test Cases ---" << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    // Test Case 1
    std::vector<double> sales1 = {1000.50, 1200.75, 1100.25, 1300.00};
    double total1 = calculateFirstQuarterSales(sales1);
    std::cout << "Test Case 1: First quarter sales: " << total1 << std::endl;

    // Test Case 2
    std::vector<double> sales2 = {2500.00, 2600.00, 2700.00, 2800.00};
    double total2 = calculateFirstQuarterSales(sales2);
    std::cout << "Test Case 2: First quarter sales: " << total2 << std::endl;

    // Test Case 3
    std::vector<double> sales3 = {99.99, 100.01, 250.50, 300.00};
    double total3 = calculateFirstQuarterSales(sales3);
    std::cout << "Test Case 3: First quarter sales: " << total3 << std::endl;

    // Test Case 4: All zero sales
    std::vector<double> sales4 = {0.0, 0.0, 0.0, 100.0};
    double total4 = calculateFirstQuarterSales(sales4);
    std::cout << "Test Case 4: First quarter sales: " << total4 << std::endl;

    // Test Case 5: Large numbers
    std::vector<double> sales5 = {1500000.78, 2300000.45, 1950000.12, 2100000.00};
    double total5 = calculateFirstQuarterSales(sales5);
    std::cout << "Test Case 5: First quarter sales: " << total5 << std::endl;
    
    return 0;
}