#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
using namespace std;

double computeArea(double radius) {
    const double PI = acos(-1.0);
    return PI * radius * radius;
}

int main() {
    vector<double> radii = {0.0, 1.0, 2.5, 10.0, 123.456};
    cout.setf(std::ios::fixed);
    cout << setprecision(6);
    for (double r : radii) {
        double area = computeArea(r);
        cout << "Radius: " << r << " -> Area: " << area << "\n";
    }
    return 0;
}