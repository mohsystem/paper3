#include <iostream>
#include <array>

// Immutable ID sequence
static const std::array<int, 5> ID_SEQUENCE = {101, 202, 303, 404, 505};

// Returns the value at the given index, or -1 if index is invalid
int getIdAtIndex(int index) {
    if (index < 0 || static_cast<size_t>(index) >= ID_SEQUENCE.size()) {
        return -1; // Sentinel for invalid index
    }
    return ID_SEQUENCE[static_cast<size_t>(index)];
}

int main() {
    int tests[5] = {0, 4, 2, -1, 10}; // 5 test cases
    for (int idx : tests) {
        std::cout << getIdAtIndex(idx) << std::endl;
    }
    return 0;
}