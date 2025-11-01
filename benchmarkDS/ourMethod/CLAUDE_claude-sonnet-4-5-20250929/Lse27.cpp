
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <limits>
#include <iomanip>

// Function to calculate total sales from three monthly sales values
double calculateTotalSales(double month1, double month2, double month3) {
    // Validate inputs are non-negative (sales cannot be negative)
    if (month1 < 0.0 || month2 < 0.0 || month3 < 0.0) {
        throw std::invalid_argument("Sales values cannot be negative");
    }
    
    // Check for NaN or infinity in inputs
    if (std::isnan(month1) || std::isnan(month2) || std::isnan(month3) ||
        std::isinf(month1) || std::isinf(month2) || std::isinf(month3)) {
        throw std::invalid_argument("Sales values must be valid finite numbers");
    }
    
    // Initialize totalSales to 0.0
    double totalSales = 0.0;
    
    // Add each month's sales to totalSales
    totalSales += month1;
    totalSales += month2;
    totalSales += month3;
    
    // Verify result is valid
    if (std::isnan(totalSales) || std::isinf(totalSales)) {
        throw std::runtime_error("Total sales calculation resulted in invalid value");
    }
    
    return totalSales;
}

int main() {
    // Set output precision for floating point
    std::cout << std::fixed << std::setprecision(2);
    
    // Test case 1: Normal positive values
    try {
        double result1 = calculateTotalSales(1000.50, 1500.75, 2000.25);
        std::cout << "Test 1 - Total sales: " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Zero values
    try {
        double result2 = calculateTotalSales(0.0, 0.0, 0.0);
        std::cout << "Test 2 - Total sales: " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Large values
    try {
        double result3 = calculateTotalSales(50000.99, 75000.50, 100000.01);
        std::cout << "Test 3 - Total sales: " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Negative value (should fail)
    try {
        double result4 = calculateTotalSales(1000.0, -500.0, 2000.0);
        std::cout << "Test 4 - Total sales: " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed (expected): " << e.what() << std::endl;
    }
    
    // Test case 5: Mixed decimal values
    try {
        double result5 = calculateTotalSales(123.45, 678.90, 234.56);
        std::cout << "Test 5 - Total sales: " << result5 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
