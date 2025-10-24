#include <iostream>
#include <cstdint>
#include <limits>

int64_t get_sum(int64_t a, int64_t b) {
    int64_t lo = (a < b) ? a : b;
    int64_t hi = (a < b) ? b : a;

    __int128 n = (__int128)hi - (__int128)lo + 1;          // number of terms
    __int128 sum = n * ((__int128)lo + (__int128)hi) / 2;  // arithmetic series sum

    if (sum > std::numeric_limits<int64_t>::max()) {
        return std::numeric_limits<int64_t>::max();
    }
    if (sum < std::numeric_limits<int64_t>::min()) {
        return std::numeric_limits<int64_t>::min();
    }
    return (int64_t)sum;
}

int main() {
    struct Test { int64_t a, b; } tests[5] = {
        {1, 0},
        {1, 2},
        {0, 1},
        {1, 1},
        {-1, 2}
    };

    for (const auto& t : tests) {
        std::cout << "get_sum(" << t.a << ", " << t.b << ") = " << get_sum(t.a, t.b) << "\n";
    }
    return 0;
}