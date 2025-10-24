#include <iostream>
#include <algorithm>
#include <climits>

long long get_sum(long long a, long long b) {
    if (a == b) return a;
    long long lo = std::min(a, b);
    long long hi = std::max(a, b);

    __int128 n = (__int128)hi - (__int128)lo + 1;
    __int128 s = (__int128)lo + (__int128)hi;

    if ((n & 1) == 0) {
        n /= 2;
    } else {
        s /= 2;
    }

    __int128 res = n * s;
    if (res > (__int128)LLONG_MAX) return LLONG_MAX;
    if (res < (__int128)LLONG_MIN) return LLONG_MIN;
    return (long long)res;
}

int main() {
    std::pair<long long, long long> tests[] = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };
    for (const auto& t : tests) {
        std::cout << get_sum(t.first, t.second) << std::endl;
    }
    return 0;
}