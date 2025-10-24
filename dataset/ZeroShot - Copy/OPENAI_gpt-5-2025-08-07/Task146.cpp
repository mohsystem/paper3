#include <iostream>
#include <vector>

int findMissing(const std::vector<int>& arr, int n) {
    if (n < 1 || static_cast<int>(arr.size()) != n - 1) {
        return -1;
    }
    std::vector<unsigned char> seen(static_cast<size_t>(n) + 1, 0);
    int x = 0;
    for (int i = 1; i <= n; ++i) {
        x ^= i;
    }
    for (int v : arr) {
        if (v < 1 || v > n) {
            return -1;
        }
        if (seen[static_cast<size_t>(v)]) {
            return -1;
        }
        seen[static_cast<size_t>(v)] = 1;
        x ^= v;
    }
    if (x < 1 || x > n || seen[static_cast<size_t>(x)]) {
        return -1;
    }
    return x;
}

int main() {
    std::vector<std::pair<std::vector<int>, int>> tests = {
        {{1, 2, 3, 5}, 5},        // -> 4
        {{}, 1},                  // -> 1
        {{7, 6, 5, 4, 3, 2}, 7},  // -> 1
        {{1, 2, 2}, 4},           // -> -1 (duplicate)
        {{0, 1, 2, 3, 4}, 6},     // -> -1 (out of range)
    };
    for (const auto& t : tests) {
        std::cout << findMissing(t.first, t.second) << "\n";
    }
    return 0;
}