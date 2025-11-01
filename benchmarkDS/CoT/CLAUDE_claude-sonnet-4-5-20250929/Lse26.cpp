
#include <iostream>
#include <stdexcept>
#include <vector>

class Lse26 {
private:
    static const std::vector<double> MONTHLY_SALES;

public:
    // Secure method to get monthly sales with input validation
    static double getMonthlySales(int month) {
        // Validate month index to prevent out-of-bounds access
        if (month < 0 || month >= static_cast<int>(MONTHLY_SALES.size())) {
            throw std::out_of_range("Invalid month index: " + std::to_string(month));
        }
        return MONTHLY_SALES[month];
    }
    
    // Calculate first quarter sales
    static double calculateFirstQuarterSales() {
        double sum = 0.0;
        
        // Iterate through first 3 months (index 0 to 2)
        for (int i = 0; i < 3; i++) {
            double monthlySale = getMonthlySales(i);
            
            // Check for valid sales value
            if (monthlySale < 0) {
                throw std::runtime_error("Sales value cannot be negative");
            }
            sum += monthlySale;
        }
        
        return sum;
    }
};

// Initialize static member
const std::vector<double> Lse26::MONTHLY_SALES = {
    15000.0, 18000.0, 16500.0, 20000.0,
    17500.0, 19000.0, 21000.0, 18500.0,
    16000.0, 22000.0, 19500.0, 23000.0
};

int main() {
    std::cout << "Test Case 1: First Quarter Sales" << std::endl;
    std::cout << "Total: $" << Lse26::calculateFirstQuarterSales() << std::endl;
    
    std::cout << "\\nTest Case 2: Individual Monthly Sales" << std::endl;
    for (int i = 0; i < 3; i++) {
        std::cout << "Month " << i << ": $" << Lse26::getMonthlySales(i) << std::endl;
    }
    
    std::cout << "\\nTest Case 3: Verify sum calculation" << std::endl;
    double manual = Lse26::getMonthlySales(0) + Lse26::getMonthlySales(1) + Lse26::getMonthlySales(2);
    std::cout << "Manual calculation: $" << manual << std::endl;
    
    std::cout << "\\nTest Case 4: Valid month access" << std::endl;
    std::cout << "Month 5 sales: $" << Lse26::getMonthlySales(5) << std::endl;
    
    std::cout << "\\nTest Case 5: Error handling" << std::endl;
    try {
        Lse26::getMonthlySales(15); // Invalid month
    } catch (const std::out_of_range& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    return 0;
}
