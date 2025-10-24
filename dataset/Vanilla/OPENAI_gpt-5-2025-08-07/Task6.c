#include <stdio.h>

double findUniq(const double* arr, int n) {
    if (n < 3) return 0.0; // simplistic guard
    double a = arr[0], b = arr[1], c = arr[2];
    double common = (a == b || a == c) ? a : b;
    for (int i = 0; i < n; ++i) {
        if (arr[i] != common) return arr[i];
    }
    return common; // Fallback
}

int main(void) {
    double t1[] = {1, 1, 1, 2, 1, 1};
    double t2[] = {0, 0, 0.55, 0, 0};
    double t3[] = {-1, -1, -1, -2, -1};
    double t4[] = {3.14, 2.71, 3.14, 3.14, 3.14};
    double t5[] = {5, 5, 5, 5, 4};

    printf("%.10g\n", findUniq(t1, (int)(sizeof(t1)/sizeof(t1[0]))));
    printf("%.10g\n", findUniq(t2, (int)(sizeof(t2)/sizeof(t2[0]))));
    printf("%.10g\n", findUniq(t3, (int)(sizeof(t3)/sizeof(t3[0]))));
    printf("%.10g\n", findUniq(t4, (int)(sizeof(t4)/sizeof(t4[0]))));
    printf("%.10g\n", findUniq(t5, (int)(sizeof(t5)/sizeof(t5[0]))));

    return 0;
}