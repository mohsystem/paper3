// Chain-of-Through process in code generation:
// 1) Problem understanding: compute remaining people on a bus given pairs [on, off].
// 2) Security requirements: validate inputs, avoid negative totals, check for overflow.
// 3) Secure coding generation: safe add/sub helpers, validate values.
// 4) Code review: ensure checks on inputs and computations.
// 5) Secure code output: final code with 5 tests.

#include <iostream>
#include <vector>
#include <utility>
#include <limits>
#include <stdexcept>

using ll = long long;

static bool safe_add_ll(ll a, ll b, ll &out) {
    if ((b > 0 && a > std::numeric_limits<ll>::max() - b) ||
        (b < 0 && a < std::numeric_limits<ll>::min() - b)) {
        return false;
    }
    out = a + b;
    return true;
}

static bool safe_sub_ll(ll a, ll b, ll &out) {
    if ((b > 0 && a < std::numeric_limits<ll>::min() + b) ||
        (b < 0 && a > std::numeric_limits<ll>::max() + b)) {
        return false;
    }
    out = a - b;
    return true;
}

// Computes remaining people on the bus after processing all stops.
// Each stop is a pair {on, off}. Returns total remaining.
ll peopleOnBus(const std::vector<std::pair<ll, ll>>& stops) {
    ll total = 0;
    for (size_t i = 0; i < stops.size(); ++i) {
        ll on = stops[i].first;
        ll off = stops[i].second;
        if (on < 0 || off < 0) {
            throw std::invalid_argument("Negative values are not allowed.");
        }
        ll temp = 0, next = 0;
        if (!safe_add_ll(total, on, temp)) {
            throw std::overflow_error("Overflow in addition.");
        }
        if (!safe_sub_ll(temp, off, next)) {
            throw std::overflow_error("Overflow in subtraction.");
        }
        if (next < 0) {
            throw std::invalid_argument("Bus population would become negative.");
        }
        total = next;
    }
    return total;
}

int main() {
    std::vector<std::pair<ll,ll>> t1 = { {10,0}, {3,5}, {5,8} }; // 5
    std::vector<std::pair<ll,ll>> t2 = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} }; // 17
    std::vector<std::pair<ll,ll>> t3 = { }; // 0
    std::vector<std::pair<ll,ll>> t4 = { {0,0} }; // 0
    std::vector<std::pair<ll,ll>> t5 = {
        {1000000000000LL, 0LL},
        {2000000000000LL, 500000000000LL},
        {3000000000000LL, 1000000000000LL}
    }; // 4500000000000

    std::vector<std::vector<std::pair<ll,ll>>> tests = { t1, t2, t3, t4, t5 };
    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            std::cout << "Test " << (i + 1) << ": " << peopleOnBus(tests[i]) << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (i + 1) << ": error - " << ex.what() << "\n";
        }
    }
    return 0;
}