
#include <iostream>
#include <cmath>
#include <random>
#include <iomanip>

class Task197 {
private:
    double radius;
    double x_center;
    double y_center;
    std::mt19937 gen;
    std::uniform_real_distribution<double> dis;
    
public:
    Task197(double radius, double x_center, double y_center) 
        : radius(radius), x_center(x_center), y_center(y_center), 
          gen(std::random_device{}()), dis(0.0, 1.0) {}
    
    double* randPoint() {
        // Use square root to ensure uniform distribution
        double r = std::sqrt(dis(gen)) * radius;
        double theta = dis(gen) * 2 * M_PI;
        
        double* result = new double[2];
        result[0] = x_center + r * std::cos(theta);
        result[1] = y_center + r * std::sin(theta);
        
        return result;
    }
};

int main() {
    // Test case 1
    Task197 solution1(1.0, 0.0, 0.0);
    std::cout << "Test case 1:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution1.randPoint();
        std::cout << std::fixed << std::setprecision(5) 
                  << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 2
    Task197 solution2(10.0, 5.0, -7.5);
    std::cout << "\\nTest case 2:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution2.randPoint();
        std::cout << std::fixed << std::setprecision(5) 
                  << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 3
    Task197 solution3(0.01, 0.0, 0.0);
    std::cout << "\\nTest case 3:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution3.randPoint();
        std::cout << std::fixed << std::setprecision(5) 
                  << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 4
    Task197 solution4(100.0, -50.0, 50.0);
    std::cout << "\\nTest case 4:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution4.randPoint();
        std::cout << std::fixed << std::setprecision(5) 
                  << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    // Test case 5
    Task197 solution5(5.5, 3.3, -2.2);
    std::cout << "\\nTest case 5:" << std::endl;
    for (int i = 0; i < 3; i++) {
        double* point = solution5.randPoint();
        std::cout << std::fixed << std::setprecision(5) 
                  << "[" << point[0] << ", " << point[1] << "]" << std::endl;
        delete[] point;
    }
    
    return 0;
}
