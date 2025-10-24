#include <iostream>
#include <vector>
#include <stdexcept>

int findOdd(const std::vector<int>& nums) {
    if (nums.empty()) {
        throw std::invalid_argument("Input array must not be empty.");
    }
    int x = 0;
    for (int v : nums) {
        x ^= v;
    }
    return x;
}

int main() {
    try {
        std::vector<int> t1{7};
        std::vector<int> t2{0};
        std::vector<int> t3{1, 1, 2};
        std::vector<int> t4{0, 1, 0, 1, 0};
        std::vector<int> t5{1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};

        std::cout << "Result 1: " << findOdd(t1) << "\n";
        std::cout << "Result 2: " << findOdd(t2) << "\n";
        std::cout << "Result 3: " << findOdd(t3) << "\n";
        std::cout << "Result 4: " << findOdd(t4) << "\n";
        std::cout << "Result 5: " << findOdd(t5) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}