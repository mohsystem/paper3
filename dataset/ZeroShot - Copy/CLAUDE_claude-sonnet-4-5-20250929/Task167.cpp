
#include <iostream>
#include <cmath>
#include <iomanip>

double cubeDiagonal(double volume) {
    // Calculate side length from volume: side = cube_root(volume)
    double side = cbrt(volume);
    
    // Calculate main diagonal: diagonal = side * sqrt(3)
    double diagonal = side * sqrt(3);
    
    // Round to 2 decimal places
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    // Test cases
    std::cout << std::fixed << std::setprecision(2);
    std::cout << cubeDiagonal(8) << std::endl;           // Expected: 3.46
    std::cout << cubeDiagonal(343) << std::endl;         // Expected: 12.12
    std::cout << cubeDiagonal(1157.625) << std::endl;    // Expected: 18.19
    std::cout << cubeDiagonal(1) << std::endl;           // Expected: 1.73
    std::cout << cubeDiagonal(27) << std::endl;          // Expected: 5.20
    
    return 0;
}
