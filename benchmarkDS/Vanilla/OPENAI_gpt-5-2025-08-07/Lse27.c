#include <stdio.h>

double computeTotalSales(double m1, double m2, double m3) {
    return m1 + m2 + m3;
}

int main(void) {
    double totalSales = 0;
    totalSales += 120.50;
    totalSales += 230.75;
    totalSales += 349.99;
    printf("%f\n", totalSales);

    totalSales = 0;
    totalSales += 0.0;
    totalSales += 0.0;
    totalSales += 0.0;
    printf("%f\n", totalSales);

    totalSales = 0;
    totalSales += 1000.00;
    totalSales += 500.25;
    totalSales += 250.25;
    printf("%f\n", totalSales);

    totalSales = 0;
    totalSales += 10.10;
    totalSales += 20.20;
    totalSales += 30.30;
    printf("%f\n", totalSales);

    totalSales = 0;
    totalSales += 99999.99;
    totalSales += 1.01;
    totalSales += 0.00;
    printf("%f\n", totalSales);

    return 0;
}