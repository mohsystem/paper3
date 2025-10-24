#include <iostream>
#include <vector>

int binaryArrayToNumber(const std::vector<int>& arr) {
    int res = 0;
    for (int bit : arr) {
        res = (res << 1) | (bit & 1);
    }
    return res;
}

void runTest(const std::vector<int>& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << arr[i];
    }
    std::cout << "] => " << binaryArrayToNumber(arr) << "\n";
}

int main() {
    runTest({0, 0, 0, 1});
    runTest({0, 0, 1, 0});
    runTest({0, 1, 0, 1});
    runTest({1, 0, 0, 1});
    runTest({1, 1, 1, 1});
    return 0;
}