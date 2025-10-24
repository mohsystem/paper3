
#include <iostream>
#include <algorithm>

int getSum(int a, int b) {
    if (a == b) {
        return a;
    }
    
    int minVal = std::min(a, b);
    int maxVal = std::max(a, b);
    
    // Using arithmetic progression formula: sum = n * (first + last) / 2
    int n = maxVal - minVal + 1;
    return n * (minVal + maxVal) / 2;
}

int main() {
    // Test case 1
    std::cout << "Test 1 (1, 0): " << getSum(1, 0) << " (Expected: 1)" << std::endl;
    
    // Test case 2
    std::cout << "Test 2 (1, 2): " << getSum(1, 2) << " (Expected: 3)" << std::endl;
    
    // Test case 3
    std::cout << "Test 3 (0, 1): " << getSum(0, 1) << " (Expected: 1)" << std::endl;
    
    // Test case 4
    std::cout << "Test 4 (1, 1): " << getSum(1, 1) << " (Expected: 1)" << std::endl;
    
    // Test case 5
    std::cout << "Test 5 (-1, 2): " << getSum(-1, 2) << " (Expected: 2)" << std::endl;
    
    return 0;
}
