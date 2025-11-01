// Chain-of-Through process in code generation:
// 1) Problem understanding: Return value at the given index from a predefined array.
// 2) Security requirements: Avoid out-of-bounds and invalid accesses.
// 3) Secure coding generation: Validate index; throw exceptions for invalid indices.
// 4) Code review: Confirm bounds checks and no undefined behavior.
// 5) Secure code output: Final code with robust checks.

#include <iostream>
#include <array>
#include <stdexcept>

static const std::array<int, 10> DATA{{10, 20, 30, 40, 50, 60, 70, 80, 90, 100}};

int valueAtIndex(int index) {
    if (index < 0 || static_cast<size_t>(index) >= DATA.size()) {
        throw std::out_of_range("Index out of bounds: " + std::to_string(index));
    }
    return DATA[static_cast<size_t>(index)];
}

int main() {
    int tests[5] = {0, 5, 9, 3, 7};
    for (int idx : tests) {
        try {
            int val = valueAtIndex(idx);
            std::cout << "Index " << idx << " -> " << val << '\n';
        } catch (const std::exception& e) {
            std::cout << "Error for index " << idx << ": " << e.what() << '\n';
        }
    }
    return 0;
}