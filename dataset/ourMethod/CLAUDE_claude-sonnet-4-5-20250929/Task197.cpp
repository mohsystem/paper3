
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#include <iomanip>

// Solution class for generating random points in a circle
class Solution {
private:
    double radius;
    double x_center;
    double y_center;
    // Security: Use proper C++11 random number generator instead of rand()
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;

public:
    // Constructor: initialize with circle parameters
    // Security: Validate input parameters to prevent invalid state
    Solution(double radius, double x_center, double y_center) 
        : radius(radius), x_center(x_center), y_center(y_center),
          gen(std::random_device{}()), dist(0.0, 1.0) {
        
        // Input validation: ensure parameters meet constraints
        if (radius <= 0.0 || radius > 1e8) {
            throw std::invalid_argument("Radius must be positive and <= 10^8");
        }
        
        if (x_center < -1e7 || x_center > 1e7 || 
            y_center < -1e7 || y_center > 1e7) {
            throw std::invalid_argument("Center coordinates must be in [-10^7, 10^7]");
        }
    }
    
    // Generate a uniform random point inside the circle
    // Security: Use cryptographically secure random if needed, bounds checked
    std::vector<double> randPoint() {
        // Generate random angle in [0, 2*PI)
        double angle = dist(gen) * 2.0 * M_PI;
        
        // Generate random radius with uniform distribution
        // Use sqrt to ensure uniform area distribution
        double r = std::sqrt(dist(gen)) * radius;
        
        // Convert polar to Cartesian coordinates
        // Bounds are safe as r <= radius and angle is normalized
        double x = x_center + r * std::cos(angle);
        double y = y_center + r * std::sin(angle);
        
        return {x, y};
    }
};

// Test driver with 5 test cases
int main() {
    std::cout << std::fixed << std::setprecision(5);
    
    std::cout << "Test Case 1: Circle with radius 1.0, center (0.0, 0.0)" << std::endl;
    try {
        Solution sol1(1.0, 0.0, 0.0);
        for (int i = 0; i < 3; i++) {
            std::vector<double> point = sol1.randPoint();
            std::cout << "  Point " << (i + 1) << ": [" 
                      << point[0] << ", " << point[1] << "]" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 2: Circle with radius 10.0, center (5.0, -7.5)" << std::endl;
    try {
        Solution sol2(10.0, 5.0, -7.5);
        for (int i = 0; i < 3; i++) {
            std::vector<double> point = sol2.randPoint();
            std::cout << "  Point " << (i + 1) << ": [" 
                      << point[0] << ", " << point[1] << "]" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 3: Circle with radius 0.01, center (0.0, 0.0)" << std::endl;
    try {
        Solution sol3(0.01, 0.0, 0.0);
        std::cout << std::setprecision(7);
        for (int i = 0; i < 3; i++) {
            std::vector<double> point = sol3.randPoint();
            std::cout << "  Point " << (i + 1) << ": [" 
                      << point[0] << ", " << point[1] << "]" << std::endl;
        }
        std::cout << std::setprecision(5);
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 4: Circle with radius 100.0, center (-50.0, 50.0)" << std::endl;
    try {
        Solution sol4(100.0, -50.0, 50.0);
        for (int i = 0; i < 3; i++) {
            std::vector<double> point = sol4.randPoint();
            std::cout << "  Point " << (i + 1) << ": [" 
                      << point[0] << ", " << point[1] << "]" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 5: Invalid input (negative radius)" << std::endl;
    try {
        Solution sol5(-1.0, 0.0, 0.0);
        std::cout << "  Should have thrown exception" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  Correctly rejected: " << e.what() << std::endl;
    }
    
    return 0;
}
