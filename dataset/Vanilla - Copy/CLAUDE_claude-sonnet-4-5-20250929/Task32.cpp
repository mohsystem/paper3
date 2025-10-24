
#include <iostream>
#include <iomanip>

class Task32 {
public:
    static int calculateYears(double principal, double interest, double tax, double desired) {
        if (principal >= desired) {
            return 0;
        }
        
        int years = 0;
        double currentAmount = principal;
        
        while (currentAmount < desired) {
            double yearInterest = currentAmount * interest;
            double taxAmount = yearInterest * tax;
            double netInterest = yearInterest - taxAmount;
            currentAmount += netInterest;
            years++;
        }
        
        return years;
    }
};

int main() {
    // Test case 1: Example from problem
    std::cout << "Test 1: " << Task32::calculateYears(1000.00, 0.05, 0.18, 1100.00) << " (Expected: 3)" << std::endl;
    
    // Test case 2: Principal equals desired
    std::cout << "Test 2: " << Task32::calculateYears(1000.00, 0.05, 0.18, 1000.00) << " (Expected: 0)" << std::endl;
    
    // Test case 3: Small growth needed
    std::cout << "Test 3: " << Task32::calculateYears(1000.00, 0.10, 0.10, 1050.00) << " (Expected: 1)" << std::endl;
    
    // Test case 4: Large growth needed
    std::cout << "Test 4: " << Task32::calculateYears(1000.00, 0.01, 0.18, 1200.00) << " (Expected: 25)" << std::endl;
    
    // Test case 5: High interest, low tax
    std::cout << "Test 5: " << Task32::calculateYears(1000.00, 0.20, 0.10, 2000.00) << " (Expected: 4)" << std::endl;
    
    return 0;
}
