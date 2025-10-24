
#include <iostream>
#include <algorithm>

int getSum(int a, int b) {
    if (a == b) {
        return a;
    }
    
    int minVal = std::min(a, b);
    int maxVal = std::max(a, b);
    
    // Sum formula: n * (first + last) / 2
    int count = maxVal - minVal + 1;
    return count * (minVal + maxVal) / 2;
}

int main() {
    std::cout << getSum(1, 0) << std::endl;    // Expected: 1
    std::cout << getSum(1, 2) << std::endl;    // Expected: 3
    std::cout << getSum(0, 1) << std::endl;    // Expected: 1
    std::cout << getSum(1, 1) << std::endl;    // Expected: 1
    std::cout << getSum(-1, 0) << std::endl;   // Expected: -1
    std::cout << getSum(-1, 2) << std::endl;   // Expected: 2
    
    return 0;
}
