#include <iostream>
#include <array>

// Step 1-5: Secure, bounds-checked retrieval from a fixed array.
int getValueAtIndex(int index) {
    static const std::array<int, 10> DATA{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int n = static_cast<int>(DATA.size());
    if (index < 0 || index >= n) {
        return -1; // out-of-bounds indicator
    }
    return DATA[static_cast<std::size_t>(index)];
}

int main() {
    int testCases[5] = {5, 0, 9, -1, 10};
    for (int i = 0; i < 5; ++i) {
        int idx = testCases[i];
        int result = getValueAtIndex(idx);
        std::cout << "Index " << idx << " -> " << result << std::endl;
    }
    return 0;
}