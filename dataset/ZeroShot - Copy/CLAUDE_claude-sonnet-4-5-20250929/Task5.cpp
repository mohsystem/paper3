
#include <iostream>

int persistence(int num) {
    if (num < 10) {
        return 0;
    }
    
    int count = 0;
    
    while (num >= 10) {
        int product = 1;
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
    std::cout << "Test 1: persistence(39) = " << persistence(39) << " (Expected: 3)" << std::endl;
    std::cout << "Test 2: persistence(999) = " << persistence(999) << " (Expected: 4)" << std::endl;
    std::cout << "Test 3: persistence(4) = " << persistence(4) << " (Expected: 0)" << std::endl;
    std::cout << "Test 4: persistence(25) = " << persistence(25) << " (Expected: 2)" << std::endl;
    std::cout << "Test 5: persistence(77) = " << persistence(77) << " (Expected: 4)" << std::endl;
    
    return 0;
}
