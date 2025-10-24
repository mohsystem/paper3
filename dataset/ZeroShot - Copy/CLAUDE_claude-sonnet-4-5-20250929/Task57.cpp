
#include <iostream>
#include <stdexcept>

long long factorial(int n) {
    if (n < 0) {
        throw std::invalid_argument("Factorial is not defined for negative numbers");
    }
    if (n == 0 || n == 1) {
        return 1;
    }
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int main() {
    // Test case 1
    std::cout << "Factorial of 0: " << factorial(0) << std::endl;
    
    // Test case 2
    std::cout << "Factorial of 1: " << factorial(1) << std::endl;
    
    // Test case 3
    std::cout << "Factorial of 5: " << factorial(5) << std::endl;
    
    // Test case 4
    std::cout << "Factorial of 10: " << factorial(10) << std::endl;
    
    // Test case 5
    std::cout << "Factorial of 15: " << factorial(15) << std::endl;
    
    return 0;
}
