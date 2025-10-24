
#include <iostream>
#include <algorithm>

class Task10 {
public:
    static int getSum(int a, int b) {
        // Handle case where a and b are equal
        if (a == b) {
            return a;
        }
        
        // Ensure min and max are correctly ordered
        int minVal = std::min(a, b);
        int maxVal = std::max(a, b);
        
        // Use arithmetic series formula: sum = n * (first + last) / 2
        // where n is the count of numbers
        int count = maxVal - minVal + 1;
        return count * (minVal + maxVal) / 2;
    }
};

int main() {
    // Test cases
    std::cout << Task10::getSum(1, 0) << std::endl;    // Expected: 1
    std::cout << Task10::getSum(1, 2) << std::endl;    // Expected: 3
    std::cout << Task10::getSum(0, 1) << std::endl;    // Expected: 1
    std::cout << Task10::getSum(1, 1) << std::endl;    // Expected: 1
    std::cout << Task10::getSum(-1, 0) << std::endl;   // Expected: -1
    std::cout << Task10::getSum(-1, 2) << std::endl;   // Expected: 2
    
    return 0;
}
