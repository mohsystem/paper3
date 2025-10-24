// Task26 - C++ implementation
// 1) Understand: Find the integer that appears an odd number of times (exactly one exists).
// 2-5) Secure coding: handle empty input; XOR reduction.
#include <iostream>
#include <vector>

int findOddOccurrence(const std::vector<int>& arr) {
    if (arr.empty()) {
        return 0; // Safe default
    }
    int x = 0;
    for (int v : arr) {
        x ^= v;
    }
    return x;
}

int main() {
    // 5 test cases
    std::cout << findOddOccurrence({7}) << "\n"; // 7
    std::cout << findOddOccurrence({0}) << "\n"; // 0
    std::cout << findOddOccurrence({1, 1, 2}) << "\n"; // 2
    std::cout << findOddOccurrence({0, 1, 0, 1, 0}) << "\n"; // 0
    std::cout << findOddOccurrence({1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}) << "\n"; // 4
    return 0;
}