
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <iomanip>

double cubeDiagonal(double volume) {
    // Input validation
    if (volume <= 0 || std::isnan(volume) || std::isinf(volume)) {
        throw std::invalid_argument("Volume must be a positive finite number");
    }
    
    // Calculate side length from volume
    double side = std::cbrt(volume);
    
    // Calculate main diagonal: side * sqrt(3)
    double diagonal = side * std::sqrt(3.0);
    
    // Round to 2 decimal places
    return std::round(diagonal * 100.0) / 100.0;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    
    // Test cases
    std::cout << "Test 1: " << cubeDiagonal(8) << std::endl;           // Expected: 3.46
    std::cout << "Test 2: " << cubeDiagonal(343) << std::endl;         // Expected: 12.12
    std::cout << "Test 3: " << cubeDiagonal(1157.625) << std::endl;    // Expected: 18.19
    std::cout << "Test 4: " << cubeDiagonal(1) << std::endl;           // Expected: 1.73
    std::cout << "Test 5: " << cubeDiagonal(27) << std::endl;          // Expected: 5.20
    
    return 0;
}
