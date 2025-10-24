
#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <stdexcept>

class Task197 {
private:
    double radius;
    double x_center;
    double y_center;
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
    
public:
    Task197(double radius, double x_center, double y_center) 
        : radius(radius), x_center(x_center), y_center(y_center), 
          gen(std::random_device{}()), dist(0.0, 1.0) {
        if (radius <= 0) {
            throw std::invalid_argument("Radius must be positive");
        }
    }
    
    std::vector<double> randPoint() {
        double angle = dist(gen) * 2 * M_PI;
        double r = std::sqrt(dist(gen)) * radius;
        
        double x = x_center + r * std::cos(angle);
        double y = y_center + r * std::sin(angle);
        
        return {x, y};
    }
};

int main() {
    Task197 solution1(1.0, 0.0, 0.0);
    auto result1 = solution1.randPoint();
    std::cout << "Test 1: [" << result1[0] << ", " << result1[1] << "]" << std::endl;
    
    auto result2 = solution1.randPoint();
    std::cout << "Test 2: [" << result2[0] << ", " << result2[1] << "]" << std::endl;
    
    auto result3 = solution1.randPoint();
    std::cout << "Test 3: [" << result3[0] << ", " << result3[1] << "]" << std::endl;
    
    Task197 solution2(5.0, 2.0, 3.0);
    auto result4 = solution2.randPoint();
    std::cout << "Test 4: [" << result4[0] << ", " << result4[1] << "]" << std::endl;
    
    auto result5 = solution2.randPoint();
    std::cout << "Test 5: [" << result5[0] << ", " << result5[1] << "]" << std::endl;
    
    return 0;
}
