#include <iostream>
#include <vector>

int longestRun(const std::vector<int>& arr) {
    if (arr.empty()) return 0;
    if (arr.size() == 1) return 1;

    int best = 1;
    int curr = 1;
    long long dir = 0; // 0 none, +1 inc, -1 dec

    for (size_t i = 1; i < arr.size(); ++i) {
        long long diff = static_cast<long long>(arr[i]) - static_cast<long long>(arr[i - 1]);
        if ((diff == 1 || diff == -1) && diff == dir) {
            curr += 1;
        } else if (diff == 1 || diff == -1) {
            curr = 2;
            dir = diff;
        } else {
            curr = 1;
            dir = 0;
        }
        if (curr > best) best = curr;
    }
    return best;
}

static void printVec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]";
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1, 2, 3, 5, 6, 7, 8, 9},
        {1, 2, 3, 10, 11, 15},
        {5, 4, 2, 1},
        {3, 5, 7, 10, 15},
        {} // empty
    };
    for (const auto& t : tests) {
        printVec(t);
        std::cout << " -> " << longestRun(t) << "\n";
    }
    return 0;
}