
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <iomanip>

double calculateCircleArea(double radius) {
    if (radius < 0) {
        throw std::invalid_argument("Radius cannot be negative");
    }
    return M_PI * radius * radius;
}

int main() {
    std::cout << std::fixed << std::setprecision(6);
    
    // Test cases
    std::cout << "Test Case 1 - Radius: 5" << std::endl;
    std::cout << "Area: " << calculateCircleArea(5) << std::endl;
    
    std::cout << "\\nTest Case 2 - Radius: 10.5" << std::endl;
    std::cout << "Area: " << calculateCircleArea(10.5) << std::endl;
    
    std::cout << "\\nTest Case 3 - Radius: 0" << std::endl;
    std::cout << "Area: " << calculateCircleArea(0) << std::endl;
    
    std::cout << "\\nTest Case 4 - Radius: 1" << std::endl;
    std::cout << "Area: " << calculateCircleArea(1) << std::endl;
    
    std::cout << "\\nTest Case 5 - Radius: 7.5" << std::endl;
    std::cout << "Area: " << calculateCircleArea(7.5) << std::endl;
    
    // Uncomment to test negative radius
    // std::cout << "\\nTest Case 6 - Radius: -5" << std::endl;
    // std::cout << "Area: " << calculateCircleArea(-5) << std::endl;
    
    return 0;
}
