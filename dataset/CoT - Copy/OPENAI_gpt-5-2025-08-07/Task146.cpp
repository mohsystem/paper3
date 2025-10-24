#include <iostream>
#include <vector>

// Secure implementation: returns -1 on invalid input
int findMissingNumber(const std::vector<int>& arr, int n) {
    if (n < 1) return -1;
    if (static_cast<int>(arr.size()) != n - 1) return -1;

    int xorFull = 0;
    for (int i = 1; i <= n; ++i) {
        xorFull ^= i;
    }

    int xorArr = 0;
    for (int v : arr) {
        xorArr ^= v;
    }

    return xorFull ^ xorArr;
}

int main() {
    // Test case 1: n=5, missing 4
    std::vector<int> t1{1, 2, 3, 5};
    std::cout << findMissingNumber(t1, 5) << std::endl; // Expected 4

    // Test case 2: n=1, missing 1 (empty array)
    std::vector<int> t2{};
    std::cout << findMissingNumber(t2, 1) << std::endl; // Expected 1

    // Test case 3: n=10, missing 7
    std::vector<int> t3{1, 2, 3, 4, 5, 6, 8, 9, 10};
    std::cout << findMissingNumber(t3, 10) << std::endl; // Expected 7

    // Test case 4: n=2, missing 1
    std::vector<int> t4{2};
    std::cout << findMissingNumber(t4, 2) << std::endl; // Expected 1

    // Test case 5: n=8, missing 8
    std::vector<int> t5{1, 2, 3, 4, 5, 6, 7};
    std::cout << findMissingNumber(t5, 8) << std::endl; // Expected 8

    return 0;
}