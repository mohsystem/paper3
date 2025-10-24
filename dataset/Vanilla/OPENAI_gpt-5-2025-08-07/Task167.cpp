#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

double cubeDiagonal(double volume) {
    double d = cbrt(volume) * sqrt(3.0);
    return round(d * 100.0) / 100.0;
}

int main() {
    double tests[] = {8, 343, 1157.625, 1, 0.125};
    for (double v : tests) {
        double res = cubeDiagonal(v);
        cout << fixed << setprecision(2) << res << "\n";
    }
    return 0;
}