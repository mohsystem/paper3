#include <iostream>
#include <climits>

long long gcd(long long a, long long b) {
    if (a == 0) {
        if (b == LLONG_MIN) return LLONG_MIN;
        return (b < 0) ? -b : b;
    }
    if (b == 0) {
        if (a == LLONG_MIN) return LLONG_MIN;
        return (a < 0) ? -a : a;
    }
    while (b != 0) {
        long long r = a % b;
        a = b;
        b = r;
    }
    if (a < 0) {
        if (a == LLONG_MIN) return LLONG_MIN;
        return -a;
    }
    return a;
}

int main() {
    long long tests[5][2] = {
        {48, 18},
        {0, 5},
        {0, 0},
        {-24, 60},
        {13, 17}
    };
    for (const auto& t : tests) {
        std::cout << "gcd(" << t[0] << ", " << t[1] << ") = " << gcd(t[0], t[1]) << "\n";
    }
    return 0;
}