
#include <iostream>
#include <string>
#include <cmath>

class Task20 {
public:
    static long digPow(int n, int p) {
        std::string digits = std::to_string(n);
        long long sum = 0;
        
        for (size_t i = 0; i < digits.length(); i++) {
            int digit = digits[i] - '0';
            sum += std::pow(digit, p + i);
        }
        
        if (sum % n == 0) {
            return sum / n;
        }
        return -1;
    }
};

int main() {
    // Test case 1
    std::cout << "Test 1: n=89, p=1 -> " << Task20::digPow(89, 1) << std::endl; // Expected: 1
    
    // Test case 2
    std::cout << "Test 2: n=92, p=1 -> " << Task20::digPow(92, 1) << std::endl; // Expected: -1
    
    // Test case 3
    std::cout << "Test 3: n=695, p=2 -> " << Task20::digPow(695, 2) << std::endl; // Expected: 2
    
    // Test case 4
    std::cout << "Test 4: n=46288, p=3 -> " << Task20::digPow(46288, 3) << std::endl; // Expected: 51
    
    // Test case 5
    std::cout << "Test 5: n=1, p=1 -> " << Task20::digPow(1, 1) << std::endl; // Expected: 1
    
    return 0;
}
