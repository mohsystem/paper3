/* Chain-of-Through steps applied: understanding, secure design, implementation, review, and safe output.
   C implementation with function and 5 test cases. */
#include <stdio.h>
#include <limits.h>

int nb_year(long long p0, double percent, long long aug, long long p) {
    if (p0 >= p) return 0;
    long double rate = percent / 100.0L;
    long long pop = p0;
    int years = 0;
    while (pop < p) {
        long long prev = pop;
        long long delta = (long long)(pop * rate); /* truncation acts as floor for non-negative rate */
        /* Check for overflow before addition */
        if ((aug > 0 && pop > LLONG_MAX - aug - delta) ||
            (aug < 0 && pop < LLONG_MIN - aug - delta)) {
            return -1; /* overflow risk -> treat as unreachable */
        }
        pop += delta + aug;
        years++;
        if (pop <= prev || years > 10000000) {
            return -1; /* unreachable or guard triggered */
        }
    }
    return years;
}

int main(void) {
    /* 5 test cases */
    printf("%d\n", nb_year(1000, 2, 50, 1200));              /* 3 */
    printf("%d\n", nb_year(1500, 5, 100, 5000));             /* 15 */
    printf("%d\n", nb_year(1500000, 2.5, 10000, 2000000));   /* 10 */
    printf("%d\n", nb_year(1500000, 0, 10000, 2000000));     /* 50 */
    printf("%d\n", nb_year(1000, 0, -1, 1200));              /* -1 */
    return 0;
}