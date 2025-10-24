/* Chain-of-Through process:
   1) Problem understanding: From cube volume V, side s = cbrt(V); main diagonal d = s * sqrt(3), round to 2 decimals.
   2) Security requirements: Validate input is finite and non-negative; avoid undefined behavior.
   3) Secure coding generation: Use cbrt, sqrt, round from math.h; check inputs.
   4) Code review: Ensured validation and rounding correctness.
   5) Secure code output: Final function with 5 test cases.
*/

#include <stdio.h>
#include <math.h>
#include <float.h>

double cubeDiagonal(double volume) {
    if (!isfinite(volume) || volume < 0.0) {
        // Indicate invalid input by returning NaN
        return NAN;
    }
    double side = cbrt(volume);
    double diagonal = side * sqrt(3.0);
    double rounded = round(diagonal * 100.0) / 100.0;
    return rounded;
}

int main(void) {
    double tests[5] = { 8.0, 343.0, 1157.625, 1.0, 0.0 };
    for (int i = 0; i < 5; ++i) {
        double v = tests[i];
        double result = cubeDiagonal(v);
        if (!isnan(result)) {
            printf("cubeDiagonal(%.6f) = %.2f\n", v, result);
        } else {
            printf("cubeDiagonal(%.6f) threw: invalid input\n", v);
        }
    }
    return 0;
}