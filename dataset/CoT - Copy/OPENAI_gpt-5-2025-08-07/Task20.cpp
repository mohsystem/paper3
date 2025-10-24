#include <iostream>
#include <string>
#include <limits>

static inline unsigned __int128 u128_max() {
    return ~((unsigned __int128)0);
}

struct PowRes {
    unsigned __int128 val;
    bool overflow;
};

static PowRes pow_u128(unsigned int base, unsigned int exp) {
    PowRes r{1, false};
    if (base == 0) {
        return PowRes{exp == 0 ? 1 : 0, false};
    }
    unsigned __int128 maxv = u128_max();
    for (unsigned int i = 0; i < exp; ++i) {
        if (r.val != 0 && r.val > maxv / base) {
            r.overflow = true;
            return r;
        }
        r.val *= base;
    }
    return r;
}

long long digPow(long long n, int p) {
    if (n <= 0 || p <= 0) return -1;
    std::string s = std::to_string(n);
    unsigned __int128 sum = 0;
    unsigned __int128 maxv = u128_max();

    for (size_t i = 0; i < s.size(); ++i) {
        int digit = s[i] - '0';
        PowRes pr = pow_u128((unsigned int)digit, (unsigned int)(p + (int)i));
        if (pr.overflow) return -1;
        if (sum > maxv - pr.val) return -1; // addition overflow
        sum += pr.val;
    }

    unsigned __int128 un = (unsigned __int128) (unsigned long long) n;
    if (un == 0) return -1;
    unsigned __int128 rem = sum % un;
    if (rem != 0) return -1;

    unsigned __int128 k = sum / un;
    if (k > (unsigned __int128)std::numeric_limits<long long>::max()) return -1;
    long long kres = (long long)k;
    return kres > 0 ? kres : -1;
}

int main() {
    std::pair<long long,int> tests[5] = {
        {89, 1},
        {92, 1},
        {695, 2},
        {46288, 3},
        {1, 1}
    };
    for (const auto &t : tests) {
        std::cout << "digPow(" << t.first << ", " << t.second << ") = " << digPow(t.first, t.second) << "\n";
    }
    return 0;
}