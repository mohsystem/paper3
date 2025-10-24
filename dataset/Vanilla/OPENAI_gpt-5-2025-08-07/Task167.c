#include <stdio.h>
#include <math.h>

double cubeDiagonal(double volume) {
    double d = cbrt(volume) * sqrt(3.0);
    return round(d * 100.0) / 100.0;
}

int main() {
    double tests[] = {8, 343, 1157.625, 1, 0.125};
    int n = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < n; i++) {
        double res = cubeDiagonal(tests[i]);
        printf("%.2f\n", res);
    }
    return 0;
}