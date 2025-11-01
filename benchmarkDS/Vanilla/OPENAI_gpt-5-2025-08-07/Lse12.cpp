#include <iostream>
#include <vector>
#include <cstdlib>

int getValueFromArray(const int* arr, int n, int idx) {
    return arr[idx];
}

int main() {
    int idx = 0;
    if (!(std::cin >> idx)) {
        idx = 0;
    }

    std::vector<std::vector<int>> tests = {
        {10, 20, 30, 40, 50},
        {1, 2, 3, 4},
        {7},
        {100, 200},
        {5, 4, 3, 2, 1, 0}
    };

    int testIdx = std::abs(idx) % static_cast<int>(tests.size());
    const std::vector<int>& selected = tests[testIdx];
    int n = static_cast<int>(selected.size());
    int safeIndex = ((idx % n) + n) % n;

    int result = getValueFromArray(selected.data(), n, safeIndex);
    return result;
}