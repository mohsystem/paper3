#include <iostream>
#include <vector>

int findMissingNumber(const std::vector<int>& arr, int n) {
    if (n < 1 || static_cast<int>(arr.size()) != n - 1) {
        return 0;
    }
    std::vector<bool> seen(static_cast<size_t>(n + 1), false); // index 0 unused
    int xorArr = 0;
    for (int v : arr) {
        if (v < 1 || v > n) {
            return 0;
        }
        if (seen[static_cast<size_t>(v)]) {
            return 0; // duplicate
        }
        seen[static_cast<size_t>(v)] = true;
        xorArr ^= v;
    }
    int xorAll = 0;
    for (int i = 1; i <= n; ++i) {
        xorAll ^= i;
    }
    int missing = xorAll ^ xorArr;
    if (missing < 1 || missing > n || seen[static_cast<size_t>(missing)]) {
        return 0;
    }
    return missing;
}

int main() {
    // Test case 1
    std::vector<int> a1{1, 2, 3, 5};
    std::cout << "Missing (n=5): " << findMissingNumber(a1, 5) << "\n"; // 4

    // Test case 2
    std::vector<int> a2{};
    std::cout << "Missing (n=1): " << findMissingNumber(a2, 1) << "\n"; // 1

    // Test case 3
    std::vector<int> a3{2, 3};
    std::cout << "Missing (n=3): " << findMissingNumber(a3, 3) << "\n"; // 1

    // Test case 4
    std::vector<int> a4{1, 2, 3, 4, 5, 6, 8, 9, 10};
    std::cout << "Missing (n=10): " << findMissingNumber(a4, 10) << "\n"; // 7

    // Test case 5 (invalid: length mismatch)
    std::vector<int> a5{1, 2, 3, 4, 5};
    std::cout << "Missing (invalid n=5, len=5): " << findMissingNumber(a5, 5) << "\n"; // 0

    return 0;
}