
#include <iostream>
#include <cstdlib>
#include <ctime>

int getMonthlySales() {
    return rand() % 100001;
}

void calculateQuarterlySales() {
    int q1Sales = 0, q2Sales = 0, q3Sales = 0, q4Sales = 0;
    
    // Quarter 1 (Jan, Feb, Mar)
    for (int i = 0; i < 3; i++) {
        q1Sales += getMonthlySales();
    }
    
    // Quarter 2 (Apr, May, Jun)
    for (int i = 0; i < 3; i++) {
        q2Sales += getMonthlySales();
    }
    
    // Quarter 3 (Jul, Aug, Sep)
    for (int i = 0; i < 3; i++) {
        q3Sales += getMonthlySales();
    }
    
    // Quarter 4 (Oct, Nov, Dec)
    for (int i = 0; i < 3; i++) {
        q4Sales += getMonthlySales();
    }
    
    std::cout << "Quarter 1 Sales: $" << q1Sales << std::endl;
    std::cout << "Quarter 2 Sales: $" << q2Sales << std::endl;
    std::cout << "Quarter 3 Sales: $" << q3Sales << std::endl;
    std::cout << "Quarter 4 Sales: $" << q4Sales << std::endl;
    std::cout << "Total Annual Sales: $" << (q1Sales + q2Sales + q3Sales + q4Sales) << std::endl;
}

int main() {
    srand(time(0));
    
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    calculateQuarterlySales();
    
    // Test case 2
    std::cout << "\\nTest Case 2:" << std::endl;
    calculateQuarterlySales();
    
    // Test case 3
    std::cout << "\\nTest Case 3:" << std::endl;
    calculateQuarterlySales();
    
    // Test case 4
    std::cout << "\\nTest Case 4:" << std::endl;
    calculateQuarterlySales();
    
    // Test case 5
    std::cout << "\\nTest Case 5:" << std::endl;
    calculateQuarterlySales();
    
    return 0;
}
