#include <iostream>
#include <array>
#include <stdexcept>

static const std::array<int, 10> DATA = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int getValueAtIndex(int index) {
    if (index < 0 || index >= static_cast<int>(DATA.size())) {
        throw std::out_of_range("Invalid index");
    }
    return DATA[static_cast<std::size_t>(index)];
}

int main() {
    std::array<int, 5> tests = {0, 5, 9, 3, 1};
    for (int t : tests) {
        try {
            int result = getValueAtIndex(t);
            std::cout << "Index " << t << " -> " << result << '\n';
        } catch (const std::exception&) {
            std::cout << "Index " << t << " -> error\n";
        }
    }
    return 0;
}