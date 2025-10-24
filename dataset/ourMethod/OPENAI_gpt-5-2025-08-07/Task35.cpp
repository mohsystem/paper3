#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <limits>
#include <cstdint>

struct Result {
    bool ok;
    long long value;
    std::string error;
};

static bool will_add_overflow_ll(long long a, long long b) {
    if (b > 0 && a > (std::numeric_limits<long long>::max() - b)) return true;
    if (b < 0 && a < (std::numeric_limits<long long>::min() - b)) return true;
    return false;
}

// Computes the number of people left on the bus after processing all stops.
// Each stop is represented as a pair {on, off}.
// Returns Result with ok=false if any validation fails (negative values, off > onboard, or overflow).
Result computePeople(const std::vector<std::pair<long long, long long>>& stops) {
    Result r{true, 0LL, ""};
    long long total = 0;

    for (size_t i = 0; i < stops.size(); ++i) {
        const long long on = stops[i].first;
        const long long off = stops[i].second;

        if (on < 0 || off < 0) {
            r.ok = false;
            r.error = "Invalid negative value at stop " + std::to_string(i);
            return r;
        }
        if (will_add_overflow_ll(total, on)) {
            r.ok = false;
            r.error = "Overflow when adding boarding at stop " + std::to_string(i);
            return r;
        }
        total += on;

        if (off > total) {
            r.ok = false;
            r.error = "More people getting off than on the bus at stop " + std::to_string(i);
            return r;
        }
        total -= off;
    }

    r.value = total;
    return r;
}

int main() {
    // 5 Test cases
    std::vector<std::vector<std::pair<long long, long long>>> tests;

    // 1) Typical example
    tests.push_back({{10, 0}, {3, 5}, {5, 8}}); // Expected: 5

    // 2) Another typical sequence
    tests.push_back({{3, 0}, {9, 1}, {4, 10}, {12, 2}}); // Expected: 15

    // 3) Empty list of stops
    tests.push_back({}); // Expected: 0

    // 4) Invalid: more off than on at a stop
    tests.push_back({{5, 0}, {2, 10}}); // Expected: error

    // 5) Large numbers within 64-bit limits
    tests.push_back({{2000000000LL, 0LL}, {2000000000LL, 0LL}, {2000000000LL, 0LL}}); // Expected: 6000000000

    for (size_t i = 0; i < tests.size(); ++i) {
        Result r = computePeople(tests[i]);
        std::cout << "Test " << i + 1 << ": ";
        if (r.ok) {
            std::cout << "People left on bus = " << r.value << "\n";
        } else {
            std::cout << "Error: " << r.error << "\n";
        }
    }

    return 0;
}