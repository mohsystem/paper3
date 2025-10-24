
#include <iostream>
#include <stdexcept>

int persistence(long long num) {
    // Input validation: ensure num is positive
    if (num < 0) {
        throw std::invalid_argument("Number must be positive");
    }
    
    // Base case: single digit
    if (num < 10) {
        return 0;
    }
    
    int count = 0;
    
    // Continue until we reach a single digit
    while (num >= 10) {
        long long product = 1;
        
        // Multiply all digits
        while (num > 0) {
            product *= num % 10;
            num /= 10;
        }
        
        num = product;
        count++;
    }
    
    return count;
}

int main() {
    // Test cases
    std::cout << "Test 1: persistence(39) = " << persistence(39) << " (expected: 3)" << std::endl;
    std::cout << "Test 2: persistence(999) = " << persistence(999) << " (expected: 4)" << std::endl;
    std::cout << "Test 3: persistence(4) = " << persistence(4) << " (expected: 0)" << std::endl;
    std::cout << "Test 4: persistence(25) = " << persistence(25) << " (expected: 2)" << std::endl;
    std::cout << "Test 5: persistence(10) = " << persistence(10) << " (expected: 1)" << std::endl;
    
    return 0;
}
