#include <iostream>
#include <vector>
#include <cmath>
#include <random>

class Solution {
private:
    double rad, x_c, y_c;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;

public:
    Solution(double radius, double x_center, double y_center)
        : rad(radius), x_c(x_center), y_c(y_center), dis(0.0, 1.0) {
        std::random_device rd;
        gen.seed(rd());
    }

    std::vector<double> randPoint() {
        const double PI = acos(-1.0);
        
        // Generate a random angle and a random length, then convert to cartesian
        // The square root is necessary for uniform distribution over the area
        double angle = dis(gen) * 2 * PI;
        double length = std::sqrt(dis(gen)) * rad;
        
        double x = x_c + length * std::cos(angle);
        double y = y_c + length * std::sin(angle);
        
        return {x, y};
    }
};

int main() {
    std::cout << "CPP Output:" << std::endl;

    // Test Case 1: Unit circle at origin
    std::cout << "\nTest Case 1: radius=1.0, center=(0.0, 0.0)" << std::endl;
    Solution sol1(1.0, 0.0, 0.0);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol1.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }

    // Test Case 2: Larger circle with offset center
    std::cout << "\nTest Case 2: radius=10.0, center=(5.0, -5.0)" << std::endl;
    Solution sol2(10.0, 5.0, -5.0);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol2.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }
    
    // Test Case 3: Small radius
    std::cout << "\nTest Case 3: radius=0.1, center=(1.0, 1.0)" << std::endl;
    Solution sol3(0.1, 1.0, 1.0);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol3.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }

    // Test Case 4: Large radius and coordinates
    std::cout << "\nTest Case 4: radius=1000.0, center=(-100.0, 200.0)" << std::endl;
    Solution sol4(1000.0, -100.0, 200.0);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol4.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }

    // Test Case 5: Zero center, different radius
    std::cout << "\nTest Case 5: radius=5.0, center=(0.0, 0.0)" << std::endl;
    Solution sol5(5.0, 0.0, 0.0);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol5.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }

    return 0;
}