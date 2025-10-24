#include <stdio.h>
#include <math.h>

double computeArea(double radius) {
    const double PI = acos(-1.0);
    return PI * radius * radius;
}

int main() {
    double radii[5] = {0.0, 1.0, 2.5, 10.0, 123.456};
    for (int i = 0; i < 5; ++i) {
        double r = radii[i];
        double area = computeArea(r);
        printf("Radius: %.6f -> Area: %.6f\n", r, area);
    }
    return 0;
}