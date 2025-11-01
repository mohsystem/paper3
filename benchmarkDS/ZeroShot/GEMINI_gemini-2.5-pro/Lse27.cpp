#include <iostream>
#include <iomanip>

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
    // Set output precision for currency-like values
    std::cout << std::fixed << std::setprecision(2);
    
    // --- 5 Test Cases ---

    // Test Case 1
    double total1 = calculateTotalSales(1200.50, 1550.75, 1300.00);
    // The last line prints the total sales to the screen.
    std::cout << "Test Case 1: Total Sales = " << total1 << std::endl;

    // Test Case 2
    double total2 = calculateTotalSales(0.0, 0.0, 0.0);
    std::cout << "Test Case 2: Total Sales = " << total2 << std::endl;

    // Test Case 3
    double total3 = calculateTotalSales(99.99, 100.01, 250.50);
    std::cout << "Test Case 3: Total Sales = " << total3 << std::endl;
    
    // Test Case 4
    double total4 = calculateTotalSales(1000000.10, 2500000.25, 1500000.15);
    std::cout << "Test Case 4: Total Sales = " << total4 << std::endl;

    // Test Case 5
    double total5 = calculateTotalSales(1.0, 2.0, 3.0);
    std::cout << "Test Case 5: Total Sales = " << total5 << std::endl;

    return 0;
}