// Chain-of-Through Process:
// 1) Problem understanding: compute remaining people on the bus after (on, off) pairs.
// 2) Security requirements: validate inputs (non-negative, off not exceeding available).
// 3) Secure coding generation: use long long for accumulation, check bounds.
// 4) Code review: ensure no negative totals and safe return.
// 5) Secure code output: function returns non-negative count or -1 on invalid input.
#include <iostream>
#include <vector>
#include <utility>
#include <limits>

int countPassengers(const std::vector<std::pair<int,int>>& stops) {
    long long total = 0LL;
    for (size_t i = 0; i < stops.size(); ++i) {
        int on = stops[i].first;
        int off = stops[i].second;
        if (on < 0 || off < 0) {
            return -1; // invalid input
        }
        long long available = total + static_cast<long long>(on);
        if (off > available) {
            return -1; // invalid transition
        }
        total = available - static_cast<long long>(off);
        if (total < 0) {
            return -1;
        }
        if (total > std::numeric_limits<int>::max()) {
            return -1; // overflow protection for return type
        }
    }
    return static_cast<int>(total);
}

int main() {
    std::vector<std::pair<int,int>> t1 = { {10,0}, {3,5}, {5,8} }; // expected 5
    std::vector<std::pair<int,int>> t2 = { }; // expected 0
    std::vector<std::pair<int,int>> t3 = { {0,0} }; // expected 0
    std::vector<std::pair<int,int>> t4 = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} }; // expected 17
    std::vector<std::pair<int,int>> t5 = { {5,0}, {0,5} }; // expected 0

    std::cout << countPassengers(t1) << "\n";
    std::cout << countPassengers(t2) << "\n";
    std::cout << countPassengers(t3) << "\n";
    std::cout << countPassengers(t4) << "\n";
    std::cout << countPassengers(t5) << "\n";
    return 0;
}