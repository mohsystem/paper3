#include <stdio.h>
#include <math.h>
#include <limits.h>

static long long safe_add_ll(long long a, long long b) {
    if (b > 0 && a > LLONG_MAX - b) return LLONG_MAX;
    if (b < 0 && a < LLONG_MIN - b) return LLONG_MIN;
    return a + b;
}

int nb_year(long long p0, double percent, long long aug, long long p) {
    if (p0 <= 0 || p <= 0 || percent < 0.0) return -1;
    if (p0 >= p) return 0;
    if (percent <= 0.0 && aug <= 0) return -1;

    long long pop = p0;
    int years = 0;
    const int MAX_YEARS = 10000000;

    while (pop < p && years < MAX_YEARS) {
        long double growth_ld = (long double)pop * ((long double)percent / 100.0L);
        long long growth;
        if (!isfinite((double)growth_ld) || growth_ld >= (long double)LLONG_MAX) {
            growth = LLONG_MAX;
        } else if (growth_ld <= (long double)LLONG_MIN) {
            growth = LLONG_MIN;
        } else {
            if (growth_ld >= 0.0L) {
                growth = (long long)floorl(growth_ld);
            } else {
                growth = (long long)ceill(growth_ld - 1.0L); // defensive, though percent >= 0 per spec
            }
        }
        long long next = safe_add_ll(pop, growth);
        next = safe_add_ll(next, aug);
        if (next == pop) return -1;
        pop = next;
        years++;
    }
    return (pop >= p) ? years : -1;
}

int main(void) {
    // 5 test cases
    printf("%d\n", nb_year(1000, 2, 50, 1200));             // expected 3
    printf("%d\n", nb_year(1500, 5, 100, 5000));            // expected 15
    printf("%d\n", nb_year(1500000, 2.5, 10000, 2000000));  // expected 10
    printf("%d\n", nb_year(1000, 0, -1, 1200));             // expected -1
    printf("%d\n", nb_year(5000, 2, 100, 3000));            // expected 0
    return 0;
}