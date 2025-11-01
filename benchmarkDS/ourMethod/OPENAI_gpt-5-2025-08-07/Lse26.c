#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stddef.h>

static double get_monthly_sales(const double* sales, size_t len, size_t i) {
    if (sales == NULL || i >= len) {
        return 0.0;
    }
    double v = sales[i];
    if (!isfinite(v)) {
        return 0.0;
    }
    return v;
}

static double total_first_quarter(const double* sales, size_t len) {
    double sum = 0.0;
    for (size_t i = 0; i < 3; i++) {
        sum += get_monthly_sales(sales, len, i);
    }
    return sum;
}

int main(void) {
    double t1[] = {100.0, 200.0, 300.0};
    double t2[] = {50.5, 60.2, 70.3};
    double t3[] = {1000.0, 2000.0, 3000.0, 4000.0};
    double t4[] = {10.0};
    double t5[] = {-5.0, 15.0, 25.0};

    double res1 = total_first_quarter(t1, sizeof(t1) / sizeof(t1[0]));
    double res2 = total_first_quarter(t2, sizeof(t2) / sizeof(t2[0]));
    double res3 = total_first_quarter(t3, sizeof(t3) / sizeof(t3[0]));
    double res4 = total_first_quarter(t4, sizeof(t4) / sizeof(t4[0]));
    double res5 = total_first_quarter(t5, sizeof(t5) / sizeof(t5[0]));

    printf("Test 1 -> Q1 total=%.2f\n", res1);
    printf("Test 2 -> Q1 total=%.2f\n", res2);
    printf("Test 3 -> Q1 total=%.2f\n", res3);
    printf("Test 4 -> Q1 total=%.2f\n", res4);
    printf("Test 5 -> Q1 total=%.2f\n", res5);

    return 0;
}