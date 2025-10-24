#include <stdio.h>
#include <math.h>
#include <float.h>

static int invalid(double x) {
    return !isfinite(x);
}

int calculateYears(double principal, double interest, double tax, double desired) {
    if (invalid(principal) || invalid(interest) || invalid(tax) || invalid(desired)) return -1;

    if (desired <= principal) return 0;

    double netRate = interest * (1.0 - tax);
    if (netRate <= 0.0) return -1;

    int years = 0;
    const int MAX_YEARS = 10000000;
    double p = principal;

    while (p < desired) {
        double interestEarned = p * interest;
        double afterTaxInterest = interestEarned * (1.0 - tax);
        p += afterTaxInterest;
        years++;
        if (years > MAX_YEARS || !isfinite(p)) return -1;
    }
    return years;
}

int main(void) {
    double tests[5][4] = {
        {1000.0, 0.05, 0.18, 1100.0},
        {1000.0, 0.05, 0.18, 1000.0},
        {1000.0, 0.0,  0.18, 1100.0},
        {1200.0, 0.0175, 0.05, 1300.0},
        {1500.0, 0.07, 0.6, 2000.0}
    };
    for (int i = 0; i < 5; ++i) {
        int years = calculateYears(tests[i][0], tests[i][1], tests[i][2], tests[i][3]);
        printf("Test %d: years = %d\n", i + 1, years);
    }
    return 0;
}