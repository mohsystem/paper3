#include <iostream>
#include <vector>
#include <optional>

std::optional<int> value_at_index(const std::vector<int>& arr, long long index) {
    if (index < 0) {
        return std::nullopt;
    }
    if (static_cast<size_t>(index) >= arr.size()) {
        return std::nullopt;
    }
    return arr[static_cast<size_t>(index)];
}

int main() {
    std::vector<int> data{10, 20, 30, 40, 50};
    std::vector<long long> testIndices{0, 2, 4, -1, 5};

    std::cout << "Array: [";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << data[i] << (i + 1 < data.size() ? ", " : "");
    }
    std::cout << "]\n";

    for (size_t i = 0; i < testIndices.size(); ++i) {
        long long idx = testIndices[i];
        auto result = value_at_index(data, idx);
        if (result.has_value()) {
            std::cout << "Test " << (i + 1) << ": index=" << idx << " -> value=" << result.value() << "\n";
        } else {
            std::cout << "Test " << (i + 1) << ": index=" << idx << " -> error\n";
        }
    }
    return 0;
}