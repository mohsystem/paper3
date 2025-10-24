#include <iostream>
#include <cstdlib>

long long gcd(long long a, long long b) {
    a = std::llabs(a);
    b = std::llabs(b);
    if (a == 0) return b;
    if (b == 0) return a;
    while (b != 0) {
        long long t = a % b;
        a = b;
        b = t;
    }
    return a;
}

int main() {
    long long tests[5][2] = {
        {54, 24},
        {0, 5},
        {17, 13},
        {-48, 18},
        {0, 0}
    };
    for (auto &t : tests) {
        std::cout << "gcd(" << t[0] << ", " << t[1] << ") = " << gcd(t[0], t[1]) << "\n";
    }
    return 0;
}