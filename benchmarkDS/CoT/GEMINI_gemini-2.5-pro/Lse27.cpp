#include <iostream>
#include <iomanip>

/**
 * @brief Calculates the total sales over three months.
 * 
 * @param month1Sales Sales for the first month.
 * @param month2Sales Sales for the second month.
 * @param month3Sales Sales for the third month.
 * @return The sum of the three months' sales.
 */
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
    // Set output to two decimal places for currency
    std::cout << std::fixed << std::setprecision(2);

    // 5 Test Cases

    // Test Case 1
    double sales1 = calculateTotalSales(1500.50, 2200.75, 1850.25);
    // The last line prints the total sales to the screen.
    std::cout << "Test Case 1 Total Sales: " << sales1 << std::endl;

    // Test Case 2
    double sales2 = calculateTotalSales(0.0, 0.0, 0.0);
    std::cout << "Test Case 2 Total Sales: " << sales2 << std::endl;
    
    // Test Case 3
    double sales3 = calculateTotalSales(99999.99, 123456.78, 87654.32);
    std::cout << "Test Case 3 Total Sales: " << sales3 << std::endl;

    // Test Case 4
    double sales4 = calculateTotalSales(100.0, 200.0, 300.0);
    std::cout << "Test Case 4 Total Sales: " << sales4 << std::endl;

    // Test Case 5
    double sales5 = calculateTotalSales(543.21, 654.32, 765.43);
    std::cout << "Test Case 5 Total Sales: " << sales5 << std::endl;
    
    return 0;
}