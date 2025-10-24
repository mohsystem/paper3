#include <stdio.h>
#include <math.h>

double cubeDiagonal(double volume) {
    if (!isfinite(volume) || volume < 0.0) {
        return NAN;
    }
    double side = cbrt(volume);
    double diag = side * sqrt(3.0);
    double rounded = round(diag * 100.0) / 100.0;
    return rounded;
}

int main(void) {
    double tests[5] = {8.0, 343.0, 1157.625, 0.0, 1.0};
    for (int i = 0; i < 5; ++i) {
        double v = tests[i];
        double res = cubeDiagonal(v);
        printf("cubeDiagonal(%.6f) = %.2f\n", v, res);
    }
    return 0;
}