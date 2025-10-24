#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through:
// 1) Problem understanding: Uniform sampling inside a circle using r = R*sqrt(U), theta in [0, 2π).
// 2) Security requirements: Validate inputs; avoid undefined behavior; use robust RNG.
// 3) Secure coding generation: Use mt19937 with random_device; check finite values.
// 4) Code review: sqrt(U) area-uniform; handle numeric stability; include circumference.
// 5) Secure code output: Clear API; no raw pointers; const correctness.

class Solution {
private:
    double radius;
    double xc;
    double yc;
    mt19937_64 rng;
    uniform_real_distribution<double> dist01; // [0,1)

    static bool is_finite(double v) {
        return std::isfinite(v);
    }

public:
    Solution(double r, double x_center, double y_center)
        : radius(r), xc(x_center), yc(y_center), dist01(0.0, 1.0) {
        if (!(r > 0.0) || !is_finite(r)) {
            throw invalid_argument("Radius must be a positive finite number.");
        }
        if (!is_finite(x_center) || !is_finite(y_center)) {
            throw invalid_argument("Center coordinates must be finite numbers.");
        }
        random_device rd;
        rng.seed(rd());
    }

    vector<double> randPoint() {
        double u = dist01(rng);                    // [0,1]
        double theta = dist01(rng) * (2.0 * M_PI); // [0,2π)
        double rr = std::sqrt(u) * radius;
        double x = xc + rr * std::cos(theta);
        double y = yc + rr * std::sin(theta);
        return {x, y};
        // Note: If u==1 due to distribution endpoints, point may be on circumference, which is allowed.
    }
};

int main() {
    // 5 test cases
    try {
        Solution s1(1.0, 0.0, 0.0);
        Solution s2(3.5, 2.0, -1.0);
        Solution s3(10.0, 1000.0, -2000.0);
        Solution s4(0.0001, -0.5, 0.5);
        Solution s5(5.0, -100.0, 100.0);

        auto p1 = s1.randPoint();
        auto p2 = s2.randPoint();
        auto p3 = s3.randPoint();
        auto p4 = s4.randPoint();
        auto p5 = s5.randPoint();

        cout.setf(std::ios::fixed); cout << setprecision(10);
        cout << "[" << p1[0] << ", " << p1[1] << "]\n";
        cout << "[" << p2[0] << ", " << p2[1] << "]\n";
        cout << "[" << p3[0] << ", " << p3[1] << "]\n";
        cout << "[" << p4[0] << ", " << p4[1] << "]\n";
        cout << "[" << p5[0] << ", " << p5[1] << "]\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}