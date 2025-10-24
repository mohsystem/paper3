#include <iostream>
#include <vector>

int findIt(const std::vector<int>& arr) {
    int xor_sum = 0;
    for (int num : arr) {
        xor_sum ^= num;
    }
    return xor_sum;
}

int main() {
    // Test cases
    std::cout << findIt({7}) << std::endl; // Output: 7
    std::cout << findIt({0}) << std::endl; // Output: 0
    std::cout << findIt({1, 1, 2}) << std::endl; // Output: 2
    std::cout << findIt({0, 1, 0, 1, 0}) << std::endl; // Output: 0
    std::cout << findIt({1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1}) << std::endl; // Output: 4
    return 0;
}