#include <iostream>
#include <vector>
#include <stdexcept>

int getValueAtIndex(int index, const std::vector<int>& arr) {
    if (index < 0 || static_cast<size_t>(index) >= arr.size()) {
        throw std::invalid_argument("invalid input");
    }
    return arr[static_cast<size_t>(index)];
}

int main() {
    std::vector<int> arr{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    std::vector<int> tests{0, 5, 9, 3, 10}; // 10 is invalid to demonstrate error handling

    for (int t : tests) {
        try {
            int value = getValueAtIndex(t, arr);
            std::cout << "index " << t << " -> " << value << "\n";
        } catch (const std::exception&) {
            std::cout << "index " << t << " -> error\n";
        }
    }
    return 0;
}