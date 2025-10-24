
#include <iostream>

long long findNb(long long m) {
    long long n = 0;
    long long sum = 0;
    
    while (sum < m) {
        n++;
        sum += n * n * n;
    }
    
    return sum == m ? n : -1;
}

int main() {
    // Test cases
    std::cout << "Test 1: findNb(1071225) = " << findNb(1071225) << std::endl; // Expected: 45
    std::cout << "Test 2: findNb(91716553919377) = " << findNb(91716553919377LL) << std::endl; // Expected: -1
    std::cout << "Test 3: findNb(4183059834009) = " << findNb(4183059834009LL) << std::endl; // Expected: 2022
    std::cout << "Test 4: findNb(24723578342962) = " << findNb(24723578342962LL) << std::endl; // Expected: -1
    std::cout << "Test 5: findNb(135) = " << findNb(135) << std::endl; // Expected: 5
    
    return 0;
}
