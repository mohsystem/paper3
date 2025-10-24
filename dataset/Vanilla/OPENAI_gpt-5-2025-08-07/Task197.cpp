#include <bits/stdc++.h>
using namespace std;

class Solution {
    double radius, x_center, y_center;
    mt19937_64 gen;
    uniform_real_distribution<double> dist;
    static double pi() { return acos(-1.0); }
public:
    Solution(double radius, double x_center, double y_center)
        : radius(radius), x_center(x_center), y_center(y_center), gen(random_device{}()), dist(0.0, 1.0) {}

    vector<double> randPoint() {
        double u = dist(gen);
        double r = sqrt(u) * radius;
        double theta = 2.0 * pi() * dist(gen);
        double x = x_center + r * cos(theta);
        double y = y_center + r * sin(theta);
        return {x, y};
    }
};

int main() {
    vector<Solution> tests;
    tests.emplace_back(1.0, 0.0, 0.0);
    tests.emplace_back(2.0, 1.0, 1.0);
    tests.emplace_back(0.5, -2.0, -3.0);
    tests.emplace_back(10.0, 5.0, -7.0);
    tests.emplace_back(3.0, 100.0, 200.0);

    cout.setf(std::ios::fixed);
    cout << setprecision(6);
    for (auto& s : tests) {
        auto p = s.randPoint();
        cout << "[" << p[0] << ", " << p[1] << "]" << '\n';
    }
    return 0;
}