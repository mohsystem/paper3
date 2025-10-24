#include <stdio.h>
#include <stddef.h>

struct Pair {
    long long on;
    long long off;
};

long long peopleOnBus(const struct Pair *stops, size_t n) {
    if (stops == NULL && n > 0) {
        return -1; /* error: invalid input pointer */
    }
    long long current = 0;
    for (size_t i = 0; i < n; ++i) {
        long long on = stops[i].on;
        long long off = stops[i].off;
        if (on < 0 || off < 0) {
            return -1; /* error: negative values not allowed */
        }
        if (i == 0 && off != 0) {
            return -1; /* error: first stop must have 0 getting off */
        }
        if (off > current + on) {
            return -1; /* error: more off than available */
        }
        current = current + on - off;
    }
    if (current < 0) {
        return -1; /* should not happen */
    }
    return current;
}

int main(void) {
    struct Pair case1[] = {{10, 0}, {3, 5}, {5, 8}}; /* Expected: 5 */
    struct Pair case2[] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}}; /* Expected: 17 */
    struct Pair case3[] = {{0, 0}}; /* Expected: 0 */
    struct Pair case4[] = {{20, 0}, {0, 10}, {5, 5}}; /* Expected: 10 */
    struct Pair case5[] = {{5, 0}, {0, 5}, {5, 0}, {0, 5}}; /* Expected: 0 */

    printf("%lld\n", peopleOnBus(case1, sizeof(case1) / sizeof(case1[0])));
    printf("%lld\n", peopleOnBus(case2, sizeof(case2) / sizeof(case2[0])));
    printf("%lld\n", peopleOnBus(case3, sizeof(case3) / sizeof(case3[0])));
    printf("%lld\n", peopleOnBus(case4, sizeof(case4) / sizeof(case4[0])));
    printf("%lld\n", peopleOnBus(case5, sizeof(case5) / sizeof(case5[0])));

    return 0;
}