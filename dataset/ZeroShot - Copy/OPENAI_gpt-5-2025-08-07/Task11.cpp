#include <iostream>
#include <cmath>
#include <limits>

static long long safe_add_ll(long long a, long long b) {
    if (b > 0 && a > std::numeric_limits<long long>::max() - b) {
        return std::numeric_limits<long long>::max();
    }
    if (b < 0 && a < std::numeric_limits<long long>::min() - b) {
        return std::numeric_limits<long long>::min();
    }
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
        long double growth_ld = static_cast<long double>(pop) * (static_cast<long double>(percent) / 100.0L);
        long long growth;
        if (!std::isfinite(growth_ld) || growth_ld >= static_cast<long double>(std::numeric_limits<long long>::max())) {
            growth = std::numeric_limits<long long>::max();
        } else if (growth_ld <= static_cast<long double>(std::numeric_limits<long long>::min())) {
            growth = std::numeric_limits<long long>::min();
        } else {
            growth = static_cast<long long>(std::floor(growth_ld));
        }
        long long next = safe_add_ll(pop, growth);
        next = safe_add_ll(next, aug);
        if (next == pop) return -1;
        pop = next;
        years++;
    }
    return (pop >= p) ? years : -1;
}

int main() {
    // 5 test cases
    std::cout << nb_year(1000, 2, 50, 1200) << "\n";             // expected 3
    std::cout << nb_year(1500, 5, 100, 5000) << "\n";            // expected 15
    std::cout << nb_year(1500000, 2.5, 10000, 2000000) << "\n";  // expected 10
    std::cout << nb_year(1000, 0, -1, 1200) << "\n";             // expected -1
    std::cout << nb_year(5000, 2, 100, 3000) << "\n";            // expected 0
    return 0;
}