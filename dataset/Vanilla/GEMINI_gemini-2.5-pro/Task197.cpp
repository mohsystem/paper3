#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Solution {
private:
    double radius;
    double x_center;
    double y_center;
    std::mt19937 gen;
    std::uniform_real_distribution<> dis;

public:
    Solution(double r, double x, double y) : radius(r), x_center(x), y_center(y), dis(0.0, 1.0) {
        std::random_device rd;
        gen.seed(rd());
    }

    std::vector<double> randPoint() {
        double angle = 2 * M_PI * dis(gen);
        double r = radius * std::sqrt(dis(gen));
        double x = x_center + r * std::cos(angle);
        double y = y_center + r * std::sin(angle);
        return {x, y};
    }
};

void printVector(const std::vector<double>& vec) {
    std::cout << std::fixed << std::setprecision(5);
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    Solution s1(1.0, 0.0, 0.0);
    printVector(s1.randPoint());
    printVector(s1.randPoint());
    printVector(s1.randPoint());
    
    Solution s2(10.0, 5.0, -7.5);
    printVector(s2.randPoint());
    printVector(s2.randPoint());
    
    Solution s3(0.01, -100.0, 100.0);
    printVector(s3.randPoint());
    
    Solution s4(1000.0, 0.0, 0.0);
    printVector(s4.randPoint());
    
    Solution s5(2.0, 2.0, 2.0);
    printVector(s5.randPoint());
    
    return 0;
}