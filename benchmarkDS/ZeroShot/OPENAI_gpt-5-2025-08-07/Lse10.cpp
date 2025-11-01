#include <iostream>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>

int getValueFromArray(const std::vector<int>& arr, std::size_t size, int index) {
    if (arr.empty() || size == 0) return 0;
    std::size_t bound = std::min<std::size_t>(size, arr.size());
    if (index >= 0 && static_cast<std::size_t>(index) < bound) {
        return arr[static_cast<std::size_t>(index)];
    }
    return 0;
}

int main() {
    std::vector<int> id_sequence{7, 13, 42, -1, 255, 1024, -300};
    std::size_t size = id_sequence.size();

    // 5 test cases
    int testIndices[5] = {0, 3, 6, -1, 100};
    for (int i = 0; i < 5; ++i) {
        int val = getValueFromArray(id_sequence, size, testIndices[i]);
        std::cout << "Test " << (i + 1) << " index=" << testIndices[i] << " value=" << val << "\n";
    }

    long long tmp = 0;
    int idx = 0;
    if (std::cin >> tmp) {
        if (tmp > std::numeric_limits<int>::max()) idx = std::numeric_limits<int>::max();
        else if (tmp < std::numeric_limits<int>::min()) idx = std::numeric_limits<int>::min();
        else idx = static_cast<int>(tmp);
    } else {
        idx = 0;
    }

    int value = getValueFromArray(id_sequence, size, idx);
    int exitCode = value & 0xFF;
    return exitCode;
}