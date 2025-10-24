
#include <iostream>
#include <string>
#include <cmath>
#include <climits>

class Task20 {
public:
    static long long digPow(long long n, int p) {
        // Input validation to prevent security issues
        if (n <= 0 || p <= 0) {
            return -1;
        }
        
        // Convert n to string to extract digits safely
        std::string nStr = std::to_string(n);
        long long sum = 0;
        
        // Calculate sum of digits raised to consecutive powers
        for (size_t i = 0; i < nStr.length(); i++) {
            // Validate character is a digit
            if (!isdigit(nStr[i])) {
                return -1;
            }
            
            int digit = nStr[i] - '0';
            int power = p + i;
            
            // Calculate power safely
            long long powResult = static_cast<long long>(std::pow(digit, power));
            
            // Check for overflow before addition
            if (sum > LLONG_MAX - powResult) {
                return -1;
            }
            sum += powResult;
        }
        
        // Check if sum is divisible by n
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
