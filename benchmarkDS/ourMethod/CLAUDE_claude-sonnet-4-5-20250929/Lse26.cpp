
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <array>

// Simulates retrieving monthly sales data
// In production, this would connect to a database or API with proper validation
double getMonthlySales(int month) {
    // Input validation: ensure month is within expected range
    if (month < 0 || month > 2) {
        // Fail closed with clear error for invalid month index
        throw std::invalid_argument("Month index must be between 0 and 2");
    }
    
    // Simulated sales data for demonstration
    // Using std::array for bounds safety
    const std::array<double, 3> salesData = {15000.50, 18500.75, 22300.25};
    
    // Additional bounds check (defensive programming)
    if (static_cast<size_t>(month) >= salesData.size()) {
        throw std::out_of_range("Month index out of range");
    }
    
    return salesData[static_cast<size_t>(month)];
}

double calculateQuarterlySales() {
    double sum = 0.0;
    
    // Iterate exactly 3 times for first quarter (months 0, 1, 2)
    for (int i = 0; i < 3; i++) {
        // Each iteration calls getMonthlySales with validated input
        double monthlySale = getMonthlySales(i);
        
        // Check for overflow before addition (defensive programming)
        if (std::isinf(sum + monthlySale)) {
            throw std::overflow_error("Sum overflow detected");
        }
        
        sum += monthlySale;
    }
    
    return sum;
}

int main() {
    // Test case 1: Normal quarterly calculation
    try {
        double result = calculateQuarterlySales();
        std::cout << "Test 1 - Quarterly Sales: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Verify individual month retrieval
    try {
        double month0 = getMonthlySales(0);
        std::cout << "Test 2 - Month 0 Sales: " << month0 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Verify boundary condition (month 2)
    try {
        double month2 = getMonthlySales(2);
        std::cout << "Test 3 - Month 2 Sales: " << month2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Invalid month index (should fail)
    try {
        double invalid = getMonthlySales(3);
        std::cout << "Test 4 - Should not reach here: " << invalid << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Expected error caught: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid negative month index (should fail)
    try {
        double invalid = getMonthlySales(-1);
        std::cout << "Test 5 - Should not reach here: " << invalid << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Expected error caught: " << e.what() << std::endl;
    }
    
    return 0;
}
