#include <bits/stdc++.h>
using namespace std;

class Solution {
private:
    double radius;
    double xCenter;
    double yCenter;
    std::random_device rd; // Non-deterministic source
public:
    Solution(double radius, double x_center, double y_center) {
        if (!(radius > 0.0) || !std::isfinite(radius)) {
            throw std::invalid_argument("Radius must be positive and finite.");
        }
        if (!std::isfinite(x_center) || !std::isfinite(y_center)) {
            throw std::invalid_argument("Center coordinates must be finite numbers.");
        }
        this->radius = radius;
        this->xCenter = x_center;
        this->yCenter = y_center;
    }

    array<double,2> randPoint() {
        std::uniform_real_distribution<double> dist01(0.0, 1.0); // [0,1)
        double u1 = dist01(rd);
        double u2 = dist01(rd);
        double r = std::sqrt(u1) * radius;
        double theta = 2.0 * M_PI * u2;
        double x = xCenter + r * std::cos(theta);
        double y = yCenter + r * std::sin(theta);
        return {x, y};
    }
};

static void printPoint(const array<double,2>& p) {
    cout.setf(std::ios::fixed); cout<<setprecision(5);
    cout << "[" << p[0] << ", " << p[1] << "]\n";
}

int main() {
    try {
        // 5 test cases (calls)
        Solution s(1.0, 0.0, 0.0);
        for (int i = 0; i < 5; ++i) {
            auto p = s.randPoint();
            printPoint(p);
        }

        // Additional example tests
        Solution s2(2.0, 3.0, -4.0);
        for (int i = 0; i < 5; ++i) {
            auto p = s2.randPoint();
            printPoint(p);
        }
    } catch (const std::exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}