
#include <iostream>
#include <iomanip>

class Task32 {
public:
    static int calculateYears(double principal, double interest, double tax, double desired) {
        if (desired <= principal) {
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
    // Test case 1: Example from prompt
    std::cout << "Test 1: " << Task32::calculateYears(1000, 0.05, 0.18, 1100) << std::endl;
    
    // Test case 2: Desired equals principal
    std::cout << "Test 2: " << Task32::calculateYears(1000, 0.05, 0.18, 1000) << std::endl;
    
    // Test case 3: Higher interest rate
    std::cout << "Test 3: " << Task32::calculateYears(1000, 0.10, 0.18, 1200) << std::endl;
    
    // Test case 4: Lower tax rate
    std::cout << "Test 4: " << Task32::calculateYears(1500, 0.06, 0.10, 2000) << std::endl;
    
    // Test case 5: Large desired amount
    std::cout << "Test 5: " << Task32::calculateYears(5000, 0.07, 0.15, 10000) << std::endl;
    
    return 0;
}
