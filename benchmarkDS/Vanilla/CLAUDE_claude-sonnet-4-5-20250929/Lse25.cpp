
#include <iostream>
#include <cstdlib>
#include <ctime>

int getMonthlySales() {
    return rand() % 100001;
}

void calculateQuarterlySales() {
    int monthlySales[12];
    
    // Get monthly sales for all 12 months
    for (int i = 0; i < 12; i++) {
        monthlySales[i] = getMonthlySales();
    }
    
    // Calculate quarterly sales
    int q1 = monthlySales[0] + monthlySales[1] + monthlySales[2];
    int q2 = monthlySales[3] + monthlySales[4] + monthlySales[5];
    int q3 = monthlySales[6] + monthlySales[7] + monthlySales[8];
    int q4 = monthlySales[9] + monthlySales[10] + monthlySales[11];
    
    // Print results
    std::cout << "Quarter 1 Sales: $" << q1 << std::endl;
    std::cout << "Quarter 2 Sales: $" << q2 << std::endl;
    std::cout << "Quarter 3 Sales: $" << q3 << std::endl;
    std::cout << "Quarter 4 Sales: $" << q4 << std::endl;
    std::cout << "Total Annual Sales: $" << (q1 + q2 + q3 + q4) << std::endl;
    std::cout << std::endl;
}

int main() {
    srand(time(0));
    
    std::cout << "Test Case 1:" << std::endl;
    calculateQuarterlySales();
    
    std::cout << "Test Case 2:" << std::endl;
    calculateQuarterlySales();
    
    std::cout << "Test Case 3:" << std::endl;
    calculateQuarterlySales();
    
    std::cout << "Test Case 4:" << std::endl;
    calculateQuarterlySales();
    
    std::cout << "Test Case 5:" << std::endl;
    calculateQuarterlySales();
    
    return 0;
}
