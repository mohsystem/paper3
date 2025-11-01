
#include <iostream>
#include <iomanip>

double calculateTotalSales(double month1, double month2, double month3) {
    double totalSales = 0.0;
    totalSales += month1;
    totalSales += month2;
    totalSales += month3;
    return totalSales;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    
    // Test case 1
    double result1 = calculateTotalSales(1000.50, 2000.75, 1500.25);
    std::cout << "Test 1 - Total Sales: $" << result1 << std::endl;
    
    // Test case 2
    double result2 = calculateTotalSales(5000.00, 3000.00, 4000.00);
    std::cout << "Test 2 - Total Sales: $" << result2 << std::endl;
    
    // Test case 3
    double result3 = calculateTotalSales(750.30, 890.60, 1200.10);
    std::cout << "Test 3 - Total Sales: $" << result3 << std::endl;
    
    // Test case 4
    double result4 = calculateTotalSales(0.00, 0.00, 0.00);
    std::cout << "Test 4 - Total Sales: $" << result4 << std::endl;
    
    // Test case 5
    double result5 = calculateTotalSales(12345.67, 23456.78, 34567.89);
    std::cout << "Test 5 - Total Sales: $" << result5 << std::endl;
    
    return 0;
}
