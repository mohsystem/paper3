#include <iostream>
#include <iomanip> // For std::fixed and std::setprecision

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
    // Setting output to fixed-point notation with 2 decimal places for currency
    std::cout << std::fixed << std::setprecision(2);

    // 5 test cases
    
    // Test Case 1
    double total1 = calculateTotalSales(1500.50, 2200.75, 1850.25);
    std::cout << "Test Case 1 Total Sales: " << total1 << std::endl;

    // Test Case 2
    double total2 = calculateTotalSales(100.0, 200.0, 300.0);
    std::cout << "Test Case 2 Total Sales: " << total2 << std::endl;

    // Test Case 3 (with zeros)
    double total3 = calculateTotalSales(0.0, 550.50, 0.0);
    std::cout << "Test Case 3 Total Sales: " << total3 << std::endl;

    // Test Case 4 (large numbers)
    double total4 = calculateTotalSales(123456.78, 987654.32, 555555.55);
    std::cout << "Test Case 4 Total Sales: " << total4 << std::endl;

    // Test Case 5 (all same values)
    double total5 = calculateTotalSales(777.77, 777.77, 777.77);
    std::cout << "Test Case 5 Total Sales: " << total5 << std::endl;
    
    return 0;
}