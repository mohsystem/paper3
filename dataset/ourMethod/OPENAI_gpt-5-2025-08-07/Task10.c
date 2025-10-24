#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int64_t get_sum(int64_t a, int64_t b) {
    int64_t lo = (a < b) ? a : b;
    int64_t hi = (a < b) ? b : a;

    __int128 n = (__int128)hi - (__int128)lo + 1;          // number of terms
    __int128 sum = n * ((__int128)lo + (__int128)hi) / 2;  // arithmetic series sum

    if (sum > (__int128)INT64_MAX) {
        return INT64_MAX;
    }
    if (sum < (__int128)INT64_MIN) {
        return INT64_MIN;
    }
    return (int64_t)sum;
}

int main(void) {
    struct Test { int64_t a, b; } tests[5] = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };

    for (size_t i = 0; i < 5; ++i) {
        int64_t res = get_sum(tests[i].a, tests[i].b);
        printf("get_sum(%" PRId64 ", %" PRId64 ") = %" PRId64 "\n", tests[i].a, tests[i].b, res);
    }
    return 0;
}