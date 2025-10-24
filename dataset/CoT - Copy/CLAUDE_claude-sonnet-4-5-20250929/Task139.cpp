
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <iomanip>

class Task139 {
private:
    static constexpr double PI = 3.14159265358979323846;

public:
    static double calculateCircleArea(double radius) {
        // Input validation
        if (radius < 0) {
            throw std::invalid_argument("Radius cannot be negative");
        }
        
        if (std::isnan(radius) || std::isinf(radius)) {
            throw std::invalid_argument("Invalid radius value");
        }
        
        // Check for potential overflow
        double max_radius = std::sqrt(std::numeric_limits<double>::max() / PI);
        if (radius > max_radius) {
            throw std::overflow_error("Radius too large, calculation would overflow");
        }
        
        // Calculate area
        double area = PI * radius * radius;
        
        if (std::isinf(area)) {
            throw std::overflow_error("Calculation resulted in overflow");
        }
        
        return area;
    }
    
    static void runTests() {
        std::cout << std::fixed << std::setprecision(6);
        
        // Test Case 1
        std::cout << "Test Case 1 - Radius: 5.0" << std::endl;
        std::cout << "Area: " << calculateCircleArea(5.0) << std::endl;
        
        // Test Case 2
        std::cout << "\\nTest Case 2 - Radius: 10.5" << std::endl;
        std::cout << "Area: " << calculateCircleArea(10.5) << std::endl;
        
        // Test Case 3
        std::cout << "\\nTest Case 3 - Radius: 0.0" << std::endl;
        std::cout << "Area: " << calculateCircleArea(0.0) << std::endl;
        
        // Test Case 4
        std::cout << "\\nTest Case 4 - Radius: 1.0" << std::endl;
        std::cout << "Area: " << calculateCircleArea(1.0) << std::endl;
        
        // Test Case 5
        std::cout << "\\nTest Case 5 - Radius: 7.5" << std::endl;
        std::cout << "Area: " << calculateCircleArea(7.5) << std::endl;
        
        // Test error case
        try {
            std::cout << "\\nTest Case 6 - Radius: -5.0 (Error case)" << std::endl;
            std::cout << "Area: " << calculateCircleArea(-5.0) << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
};

int main() {
    Task139::runTests();
    return 0;
}
