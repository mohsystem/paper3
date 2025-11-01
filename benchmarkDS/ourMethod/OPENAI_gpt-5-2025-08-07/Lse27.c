#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static long long add_exact_ll(long long a, long long b) {
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
        fputs("error\n", stderr);
        exit(1);
    }
    return a + b;
}

long long compute_total_sales(long long month1, long long month2, long long month3) {
    long long tmp = add_exact_ll(month1, month2);
    return add_exact_ll(tmp, month3);
}

int main(void) {
    long long totalSales = 0;
    totalSales = add_exact_ll(totalSales, 100);
    totalSales = add_exact_ll(totalSales, 200);
    totalSales = add_exact_ll(totalSales, 300);
    printf("Total Sales: %lld\n", totalSales);

    {
        long long totalSales2 = 0;
        totalSales2 = add_exact_ll(totalSales2, 0);
        totalSales2 = add_exact_ll(totalSales2, 0);
        totalSales2 = add_exact_ll(totalSales2, 0);
        printf("Total Sales: %lld\n", totalSales2);
    }

    {
        long long totalSales3 = 0;
        totalSales3 = add_exact_ll(totalSales3, 999);
        totalSales3 = add_exact_ll(totalSales3, 1);
        totalSales3 = add_exact_ll(totalSales3, 0);
        printf("Total Sales: %lld\n", totalSales3);
    }

    {
        long long totalSales4 = 0;
        totalSales4 = add_exact_ll(totalSales4, 12345);
        totalSales4 = add_exact_ll(totalSales4, 67890);
        totalSales4 = add_exact_ll(totalSales4, 111);
        printf("Total Sales: %lld\n", totalSales4);
    }

    {
        long long totalSales5 = 0;
        totalSales5 = add_exact_ll(totalSales5, -50);
        totalSales5 = add_exact_ll(totalSales5, 100);
        totalSales5 = add_exact_ll(totalSales5, 200);
        printf("Total Sales: %lld\n", totalSales5);
    }

    return 0;
}