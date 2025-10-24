#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <limits>

class Solution {
public:
    Solution(double radius, double x_center, double y_center)
        : r(radius), xc(x_center), yc(y_center) {
        if (!std::isfinite(radius) || !std::isfinite(x_center) || !std::isfinite(y_center)) {
            throw std::invalid_argument("Inputs must be finite numbers.");
        }
        if (radius <= 0.0) {
            throw std::invalid_argument("Radius must be > 0.");
        }
        std::random_device rd;
        auto seed1 = rd();
        auto seed2 = rd();
        std::seed_seq seq{seed1, seed2, static_cast<unsigned>(reinterpret_cast<std::uintptr_t>(this))};
        gen.seed(seq);
        dist01 = std::uniform_real_distribution<double>(0.0, 1.0);
    }

    std::vector<double> randPoint() {
        double u = dist01(gen);
        double v = dist01(gen);
        double rad = std::sqrt(u) * r;
        double theta = 2.0 * M_PI * v;
        double x = xc + rad * std::cos(theta);
        double y = yc + rad * std::sin(theta);
        return {x, y};
    }

private:
    double r, xc, yc;
    std::mt19937_64 gen;
    std::uniform_real_distribution<double> dist01;
};

int main() {
    try {
        // 5 test cases
        Solution s1(1.0, 0.0, 0.0);
        Solution s2(2.5, 3.0, -4.0);
        Solution s3(10.0, -100.0, 100.0);
        Solution s4(0.5, 1e5, -1e5);
        Solution s5(1e8, 1e7, -1e7);

        auto p1 = s1.randPoint();
        auto p2 = s2.randPoint();
        auto p3 = s3.randPoint();
        auto p4 = s4.randPoint();
        auto p5 = s5.randPoint();

        std::cout << "Test 1: [" << p1[0] << ", " << p1[1] << "]\n";
        std::cout << "Test 2: [" << p2[0] << ", " << p2[1] << "]\n";
        std::cout << "Test 3: [" << p3[0] << ", " << p3[1] << "]\n";
        std::cout << "Test 4: [" << p4[0] << ", " << p4[1] << "]\n";
        std::cout << "Test 5: [" << p5[0] << ", " << p5[1] << "]\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}