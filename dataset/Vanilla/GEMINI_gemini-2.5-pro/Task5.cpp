#include <iostream>

int persistence(long long num) {
    int count = 0;
    while (num >= 10) {
        count++;
        long long product = 1;
        long long tempNum = num;
        while (tempNum > 0) {
            product *= tempNum % 10;
            tempNum /= 10;
        }
        num = product;
    }
    return count;
}

int main() {
    // Test cases
    std::cout << "Input: 39, Output: " << persistence(39) << std::endl;   // Expected: 3
    std::cout << "Input: 999, Output: " << persistence(999) << std::endl; // Expected: 4
    std::cout << "Input: 4, Output: " << persistence(4) << std::endl;     // Expected: 0
    std::cout << "Input: 25, Output: " << persistence(25) << std::endl;   // Expected: 2
    std::cout << "Input: 679, Output: " << persistence(679) << std::endl; // Expected: 5
    return 0;
}