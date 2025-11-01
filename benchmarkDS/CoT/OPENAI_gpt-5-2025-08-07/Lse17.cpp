// Chain-of-Through process:
// 1) Problem understanding: Safely retrieve value at a given index in a vector.
// 2) Security requirements: Prevent out-of-bounds access, handle empty/null-like cases.
// 3) Secure coding generation: Validate index before accessing.
// 4) Code review: Ensured bounds checks and no undefined behavior.
// 5) Secure code output: Uses std::optional<int> to signal presence/absence of a valid value.
#include <iostream>
#include <vector>
#include <optional>

std::optional<int> getValueAtIndex(const std::vector<int>& arr, int index) {
    if (index < 0 || static_cast<size_t>(index) >= arr.size()) {
        return std::nullopt;
    }
    return arr[static_cast<size_t>(index)];
}

int main() {
    std::vector<int> data{10, 20, 30, 40, 50};
    std::vector<int> testIndices{0, 2, 4, -1, 10};

    for (int idx : testIndices) {
        auto result = getValueAtIndex(data, idx);
        if (result.has_value()) {
            std::cout << "Index " << idx << " -> " << result.value() << "\n";
        } else {
            std::cout << "Index " << idx << " -> invalid index\n";
        }
    }
    return 0;
}