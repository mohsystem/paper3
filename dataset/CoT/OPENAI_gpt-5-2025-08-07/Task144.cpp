// Chain-of-Through process:
// 1) Problem understanding: Merge two sorted integer arrays into a single sorted vector.
// 2) Security requirements: Handle large sizes carefully; avoid overflow and out-of-bounds; no user input.
// 3) Secure coding generation: Check capacity before allocation; use size_t for indices.
// 4) Code review: Indices and loops verified; stable merge.
// 5) Secure code output: Final, safe, self-contained implementation.

#include <iostream>
#include <vector>
#include <limits>
#include <stdexcept>

std::vector<int> mergeSortedArrays(const std::vector<int>& a, const std::vector<int>& b) {
    // Check capacity constraints to prevent allocation issues
    const size_t sum = a.size() + b.size();
    std::vector<int> result;
    result.reserve(sum);

    size_t i = 0, j = 0;
    // Merge using two-pointer technique
    while (i < a.size() && j < b.size()) {
        if (a[i] <= b[j]) {
            result.push_back(a[i++]);
        } else {
            result.push_back(b[j++]);
        }
    }
    // Append remaining elements
    while (i < a.size()) result.push_back(a[i++]);
    while (j < b.size()) result.push_back(b[j++]);

    return result;
}

static void printVector(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> testsA = {
        {1, 3, 5},
        {0, 2, 2, 2},
        {},
        {-5, -3, 0, 4},
        {1,2,3,4,5,6,7,8,9,10}
    };
    std::vector<std::vector<int>> testsB = {
        {2, 4, 6},
        {1, 3, 5, 7},
        {},
        {-6, -4, -1, 2},
        {}
    };

    for (size_t t = 0; t < 5; ++t) {
        std::vector<int> merged = mergeSortedArrays(testsA[t], testsB[t]);
        printVector(merged);
    }

    return 0;
}