#include <iostream>
#include <vector>
#include <stdexcept>

int getValueAtIndex(const std::vector<int>& arr, long long index) {
    if (index < 0) {
        throw std::out_of_range("Index out of bounds.");
    }
    long long n = static_cast<long long>(arr.size());
    if (index >= n) {
        throw std::out_of_range("Index out of bounds.");
    }
    return arr[static_cast<size_t>(index)];
}

int main() {
    std::vector<int> arr{10, 20, 30, 40, 50};
    long long tests[5] = {0, 2, 4, 5, -1};

    std::cout << "Array: [";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << (i + 1 < arr.size() ? ", " : "");
    }
    std::cout << "]\n";

    for (long long idx : tests) {
        try {
            int value = getValueAtIndex(arr, idx);
            std::cout << "Index " << idx << " -> " << value << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Index " << idx << " -> Error: " << ex.what() << "\n";
        }
    }
    return 0;
}