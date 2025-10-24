
#include <iostream>
#include <cmath>
#include <climits>

class Task23 {
public:
    static long long findNb(long long m) {
        // Input validation
        if (m <= 0) {
            return -1;
        }
        
        long long n = 0;
        long long sum = 0;
        
        // Iterate until sum equals or exceeds m
        while (sum < m) {
            n++;
            // Check for potential overflow
            if (n > cbrt(LLONG_MAX)) {
                return -1;
            }
            long long cube = n * n * n;
            // Check if adding cube would overflow
            if (sum > LLONG_MAX - cube) {
                return -1;
            }
            sum += cube;
        }
        
        // Return n if exact match, otherwise -1
        return (sum == m) ? n : -1;
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Task23::findNb(1071225) << " (Expected: 45)" << std::endl;
    std::cout << "Test 2: " << Task23::findNb(91716553919377LL) << " (Expected: -1)" << std::endl;
    std::cout << "Test 3: " << Task23::findNb(4183059834009LL) << " (Expected: 2022)" << std::endl;
    std::cout << "Test 4: " << Task23::findNb(24723578342962LL) << " (Expected: -1)" << std::endl;
    std::cout << "Test 5: " << Task23::findNb(135) << " (Expected: 5)" << std::endl;
    
    return 0;
}
