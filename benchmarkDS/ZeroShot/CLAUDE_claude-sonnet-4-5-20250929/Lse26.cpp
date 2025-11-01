
#include <iostream>
#include <iomanip>
#include <vector>

class Lse26 {
public:
    static double getMonthlySales(int month) {
        // Simulated monthly sales data (in a real application, this might come from a database)
        static const std::vector<double> salesData = {
            15000.50, 22000.75, 18500.25, 25000.00, 30000.50, 
            27000.75, 32000.25, 28000.50, 24000.75, 26000.25, 
            31000.50, 35000.75
        };
        
        // Validate month index
        if (month >= 0 && month < static_cast<int>(salesData.size())) {
            return salesData[month];
        }
        return 0.0;
    }
    
    static double calculateFirstQuarterSales() {
        double sum = 0.0;
        
        for (int i = 0; i < 3; i++) {
            sum += getMonthlySales(i);
        }
        
        return sum;
    }
};

int main() {
    std::cout << std::fixed << std::setprecision(2);
    
    // Test case 1: Calculate first quarter sales
    std::cout << "Test Case 1 - First Quarter Sales: $" 
              << Lse26::calculateFirstQuarterSales() << std::endl;
    
    // Test case 2: Verify individual months
    std::cout << "\\nTest Case 2 - Individual Month Sales:" << std::endl;
    for (int i = 0; i < 3; i++) {
        std::cout << "Month " << i << ": $" << Lse26::getMonthlySales(i) << std::endl;
    }
    
    // Test case 3: Manual calculation verification
    std::cout << "\\nTest Case 3 - Manual Verification:" << std::endl;
    double manual = Lse26::getMonthlySales(0) + Lse26::getMonthlySales(1) + Lse26::getMonthlySales(2);
    std::cout << "Manual calculation: $" << manual << std::endl;
    
    // Test case 4: Check with different starting data
    std::cout << "\\nTest Case 4 - Second Quarter Sales:" << std::endl;
    double q2Sum = 0.0;
    for (int i = 3; i < 6; i++) {
        q2Sum += Lse26::getMonthlySales(i);
    }
    std::cout << "Second Quarter Sales: $" << q2Sum << std::endl;
    
    // Test case 5: Validate boundary conditions
    std::cout << "\\nTest Case 5 - Boundary Conditions:" << std::endl;
    std::cout << "Invalid month (-1): $" << Lse26::getMonthlySales(-1) << std::endl;
    std::cout << "Invalid month (12): $" << Lse26::getMonthlySales(12) << std::endl;
    std::cout << "Valid month (0): $" << Lse26::getMonthlySales(0) << std::endl;
    
    return 0;
}
