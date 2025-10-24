#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

int nb_year(long long p0, double percent, long long aug, long long p) {
    // Validate inputs
    if (p0 <= 0 || p <= 0 || percent < 0.0 || !isfinite(percent)) {
        return -1;
    }
    long long cur = p0;
    if (cur >= p) return 0;

    const int maxYears = 10000000; // Safety cap
    int years = 0;

    while (cur < p) {
        if (years >= maxYears) return -1;

        double growth = (double)cur * (percent / 100.0);
        if (!isfinite(growth)) return -1;

        double baseD = floor((double)cur + growth);
        if (!isfinite(baseD)) return -1;

        if (baseD > (double)LLONG_MAX || baseD < (double)LLONG_MIN) {
            return -1;
        }

        long long baseLL = (long long)baseD;

        // Check overflow on addition with aug
        if ((aug > 0 && baseLL > LLONG_MAX - aug) ||
            (aug < 0 && baseLL < LLONG_MIN - aug)) {
            return -1;
        }

        long long next = baseLL + aug;
        if (next < 0) next = 0; // Clamp to non-negative population

        years++;
        if (next == cur) return -1; // Stagnation -> impossible to reach target
        cur = next;
    }

    return years;
}

int main(void) {
    // 5 test cases
    printf("Test 1: %d (expected 3)\n", nb_year(1000, 2.0, 50, 1200));
    printf("Test 2: %d (expected 15)\n", nb_year(1500, 5.0, 100, 5000));
    printf("Test 3: %d (expected 10)\n", nb_year(1500000, 2.5, 10000, 2000000));
    printf("Test 4: %d (expected 0)\n", nb_year(1500, 0.0, 0, 1500));
    printf("Test 5: %d (expected -1)\n", nb_year(1000, 0.0, 0, 2000));
    return 0;
}