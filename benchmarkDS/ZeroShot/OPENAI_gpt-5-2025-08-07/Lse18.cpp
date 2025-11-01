#include <iostream>
#include <stdexcept>
#include <array>

class Lse18 {
public:
    static int getValueAtIndex(int index) {
        if (index < 0 || index >= static_cast<int>(DATA.size())) {
            throw std::out_of_range("index out of bounds");
        }
        return DATA[static_cast<std::size_t>(index)];
    }
private:
    static const std::array<int, 10> DATA;
};

const std::array<int, 10> Lse18::DATA = {11, 22, 33, 44, 55, 66, 77, 88, 99, 110};

int main() {
    int tests[5] = {0, 5, 9, 3, 7};
    for (int t : tests) {
        try {
            int v = Lse18::getValueAtIndex(t);
            std::cout << "Index " << t << " -> " << v << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Index " << t << " -> error: " << e.what() << std::endl;
        }
    }
    return 0;
}