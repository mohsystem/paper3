#include <iostream>
#include <stdexcept>
#include <array>

class Lse16 {
public:
    static int getValueAtIndex(int index) {
        constexpr std::array<int, 8> DATA = {10, 20, 30, 40, 50, 60, 70, 80};
        if (index < 0 || index >= static_cast<int>(DATA.size())) {
            throw std::out_of_range("index out of range");
        }
        return DATA[static_cast<std::size_t>(index)];
    }
};

int main() {
    int tests[5] = {0, 1, 2, 5, 7};
    for (int t : tests) {
        try {
            int val = Lse16::getValueAtIndex(t);
            std::cout << "Index " << t << " -> " << val << '\n';
        } catch (const std::exception& e) {
            std::cout << "Index " << t << " -> error: " << e.what() << '\n';
        }
    }
    return 0;
}