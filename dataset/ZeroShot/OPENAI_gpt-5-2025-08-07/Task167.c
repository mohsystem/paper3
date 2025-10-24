#include <stdio.h>
#include <math.h>
#include <float.h>

double cubeDiagonal(double volume) {
    if (!isfinite(volume) || volume < 0.0) {
        return NAN;
    }
    double diag = cbrt(volume) * sqrt(3.0);
    double rounded = round(diag * 100.0) / 100.0;
    return rounded;
}

int main(void) {
    double tests[5] = {8.0, 343.0, 1157.625, 0.0, -5.0};
    for (int i = 0; i < 5; ++i) {
        double v = tests[i];
        double res = cubeDiagonal(v);
        printf("cubeDiagonal(%.6g) = ", v);
        if (isnan(res)) {
            printf("NaN\n");
        } else {
            printf("%.2f\n", res);
        }
    }
    return 0;
}