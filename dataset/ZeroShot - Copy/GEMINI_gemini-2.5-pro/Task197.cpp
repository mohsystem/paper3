#include <iostream>
#include <vector>
#include <cmath>
#include <random>

// Define M_PI if it's not defined by the compiler
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Solution {
private:
    double m_radius, m_x_center, m_y_center;
    std::mt19937 m_gen;
    std::uniform_real_distribution<> m_dis;

public:
    /**
     * @brief Initializes the object with the circle's properties.
     * @param radius The radius of the circle.
     * @param x_center The x-coordinate of the center.
     * @param y_center The y-coordinate of the center.
     */
    Solution(double radius, double x_center, double y_center)
        : m_radius(radius), m_x_center(x_center), m_y_center(y_center), 
          m_gen(std::random_device{}()), m_dis(0.0, 1.0) {}

    /**
     * @brief Generates a uniform random point inside the circle.
     * @return A vector containing the [x, y] coordinates of the point.
     */
    std::vector<double> randPoint() {
        // Use polar coordinates with inverse transform sampling for uniform distribution
        double r = m_radius * std::sqrt(m_dis(m_gen)); // For uniform distribution of area
        double theta = m_dis(m_gen) * 2.0 * M_PI;

        double x = m_x_center + r * std::cos(theta);
        double y = m_y_center + r * std::sin(theta);
        
        return {x, y};
    }
};

void run_test_case(const std::string& name, double r, double xc, double yc) {
    std::cout << "\n" << name << " (radius=" << r << ", center=(" << xc << ", " << yc << ")):" << std::endl;
    Solution sol(r, xc, yc);
    for (int i = 0; i < 5; ++i) {
        std::vector<double> p = sol.randPoint();
        std::cout << "[" << p[0] << ", " << p[1] << "]" << std::endl;
    }
}

int main() {
    std::cout.precision(15);
    run_test_case("Test Case 1", 1.0, 0.0, 0.0);
    run_test_case("Test Case 2", 10.0, 5.0, -7.5);
    run_test_case("Test Case 3", 0.1, 100.0, 100.0);
    run_test_case("Test Case 4", 1e8, -1e7, 1e7);
    run_test_case("Test Case 5", 1.0, 1.0, 1.0);
    return 0;
}