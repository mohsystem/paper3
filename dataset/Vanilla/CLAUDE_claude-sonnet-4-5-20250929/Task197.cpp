
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>

class Solution {
private:
    double radius;
    double x_center;
    double y_center;
    
    double randomDouble() {
        return (double)rand() / RAND_MAX;
    }
    
public:
    Solution(double radius, double x_center, double y_center) {
        this->radius = radius;
        this->x_center = x_center;
        this->y_center = y_center;
        srand(time(0));
    }
    
    double* randPoint() {
        // Use square root to ensure uniform distribution
        double r = sqrt(randomDouble()) * radius;
        double theta = randomDouble() * 2 * M_PI;
        
        double* point = new double[2];
        point[0] = x_center + r * cos(theta);
        point[1] = y_center + r * sin(theta);
        
        return point;
    }
};

int main() {
    // Test case 1
    Solution solution1(1.0, 0.0, 0.0);
    std::cout << "Test 1:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution1.randPoint();
        std::cout << std::fixed << std::setprecision(5) << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 2
    Solution solution2(10.0, 5.0, -7.5);
    std::cout << "\\nTest 2:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution2.randPoint();
        std::cout << std::fixed << std::setprecision(5) << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 3
    Solution solution3(0.01, 0.0, 0.0);
    std::cout << "\\nTest 3:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution3.randPoint();
        std::cout << std::fixed << std::setprecision(5) << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 4
    Solution solution4(100.0, -50.0, 50.0);
    std::cout << "\\nTest 4:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution4.randPoint();
        std::cout << std::fixed << std::setprecision(5) << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 5
    Solution solution5(5.0, 10.0, 10.0);
    std::cout << "\\nTest 5:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution5.randPoint();
        std::cout << std::fixed << std::setprecision(5) << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    return 0;
}
