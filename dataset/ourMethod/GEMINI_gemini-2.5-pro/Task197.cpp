#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Solution {
private:
    double m_radius;
    double m_x_center;
    double m_y_center;
    std::mt19937 m_gen;
    std::uniform_real_distribution<> m_dist;

public:
    Solution(double radius, double x_center, double y_center)
        : m_radius(radius), m_x_center(x_center), m_y_center(y_center), m_dist(0.0, 1.0) {
        std::random_device rd;
        m_gen.seed(rd());
    }
    
    std::vector<double> randPoint() {
        double angle = m_dist(m_gen) * 2 * M_PI;
        double r = m_radius * sqrt(m_dist(m_gen));
        
        double x = m_x_center + r * cos(angle);
        double y = m_y_center + r * sin(angle);
        
        return {x, y};
    }
};

void run_test_case(const std::string& name, double r, double x, double y) {
    std::cout << name << " (R=" << r << ", C=(" << x << "," << y << ")):" << std::endl;
    Solution sol(r, x, y);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout.precision(15);
    
    run_test_case("Test Case 1", 1.0, 0.0, 0.0);
    run_test_case("Test Case 2", 10.0, 5.0, -5.0);
    run_test_case("Test Case 3", 0.1, 100.0, 100.0);
    run_test_case("Test Case 4", 1000.0, -500.0, 200.0);
    run_test_case("Test Case 5", 5.0, 0.0, 0.0);
    
    return 0;
}