#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <limits>

std::optional<int> get_value_at_index(const std::vector<int>& id_sequence, std::size_t index) {
    if (index >= id_sequence.size()) {
        return std::nullopt;
    }
    return id_sequence[index];
}

std::optional<int> prompt_and_get_value(const std::vector<int>& id_sequence) {
    std::cout << "Enter index (0.." << (id_sequence.empty() ? 0 : id_sequence.size() - 1) << "): ";
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "No input.\n";
        return std::nullopt;
    }
    if (line.size() == 0 || line.size() > 18) {
        std::cout << "Invalid input.\n";
        return std::nullopt;
    }
    try {
        std::size_t pos = 0;
        long long v = std::stoll(line, &pos, 10);
        if (pos != line.size() || v < 0) {
            std::cout << "Invalid input.\n";
            return std::nullopt;
        }
        std::size_t idx = static_cast<std::size_t>(v);
        auto val = get_value_at_index(id_sequence, idx);
        if (!val.has_value()) {
            std::cout << "Invalid index.\n";
            return std::nullopt;
        }
        return val;
    } catch (...) {
        std::cout << "Invalid input.\n";
        return std::nullopt;
    }
}

int main() {
    std::vector<int> id_sequence {42, 7, 13, 99, 5, 81, 23, 64, 12, 30};
    for (int t = 0; t < 5; ++t) {
        auto result = prompt_and_get_value(id_sequence);
        if (result.has_value()) {
            std::cout << "Value: " << *result << "\n";
        } else {
            std::cout << "No value returned.\n";
        }
    }
    return 0;
}