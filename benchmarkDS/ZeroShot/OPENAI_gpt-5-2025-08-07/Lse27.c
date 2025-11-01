#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static int safe_add_ll(long long a, long long b, long long* out) {
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) {
        return 0;
    }
    *out = a + b;
    return 1;
}

static long long checked_add(long long a, long long b) {
    long long out = 0;
    if (!safe_add_ll(a, b, &out)) {
        fputs("Overflow during addition\n", stderr);
        exit(EXIT_FAILURE);
    }
    return out;
}

long long compute_total_sales(long long m1, long long m2, long long m3) {
    long long total = 0LL;
    total = checked_add(total, m1);
    total = checked_add(total, m2);
    total = checked_add(total, m3);
    return total;
}

int main(void) {
    long long totalSales = 0LL;
    totalSales = checked_add(totalSales, 100LL);
    totalSales = checked_add(totalSales, 200LL);
    totalSales = checked_add(totalSales, 300LL);
    printf("%lld\n", totalSales);

    totalSales = 0LL;
    totalSales = checked_add(totalSales, 4567LL);
    totalSales = checked_add(totalSales, 8901LL);
    totalSales = checked_add(totalSales, 2345LL);
    printf("%lld\n", totalSales);

    totalSales = 0LL;
    totalSales = checked_add(totalSales, 0LL);
    totalSales = checked_add(totalSales, 999999999LL);
    totalSales = checked_add(totalSales, 1LL);
    printf("%lld\n", totalSales);

    totalSales = 0LL;
    totalSales = checked_add(totalSales, 1000000000000LL);
    totalSales = checked_add(totalSales, 2000000000000LL);
    totalSales = checked_add(totalSales, 3000000000000LL);
    printf("%lld\n", totalSales);

    totalSales = 0LL;
    totalSales = checked_add(totalSales, 123456789012345LL);
    totalSales = checked_add(totalSales, 987654321098765LL);
    totalSales = checked_add(totalSales, 111111111111111LL);
    printf("%lld\n", totalSales);

    return 0;
}