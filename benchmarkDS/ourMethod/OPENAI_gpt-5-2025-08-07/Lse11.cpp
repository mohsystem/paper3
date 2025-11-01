#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

int getValueFromArray(const std::vector<int>& arr, std::size_t size, int index) {
    if (size > arr.size()) {
        throw std::invalid_argument("Invalid size");
    }
    if (index < 0 || static_cast<std::size_t>(index) >= size) {
        throw std::out_of_range("Invalid index");
    }
    return arr[static_cast<std::size_t>(index)];
}

int main() {
    const std::vector<int> id_sequence{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    const std::size_t size = id_sequence.size();

    // 5 test cases
    const int testIndices[5] = {0, 3, 9, 5, 7};
    for (int idx : testIndices) {
        try {
            int val = getValueFromArray(id_sequence, size, idx);
            std::cout << "Test index " << idx << " => " << val << "\n";
        } catch (...) {
            std::cout << "Test index " << idx << " => ERROR\n";
        }
    }

    // Read one integer index from stdin and return value as exit code
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cerr << "ERROR\n";
        return 1;
    }
    if (line.empty() || line.size() > 64) {
        std::cerr << "ERROR\n";
        return 1;
    }
    try {
        std::size_t pos = 0;
        long long idxll = std::stoll(line, &pos, 10);
        if (pos != line.find_last_not_of(" \t\r\n") + 1) {
            std::cerr << "ERROR\n";
            return 1;
        }
        if (idxll < std::numeric_limits<int>::min() || idxll > std::numeric_limits<int>::max()) {
            std::cerr << "ERROR\n";
            return 1;
        }
        int index = static_cast<int>(idxll);
        if (index < 0 || static_cast<std::size_t>(index) >= size) {
            std::cerr << "ERROR\n";
            return 1;
        }
        int result = getValueFromArray(id_sequence, size, index);
        return result;
    } catch (...) {
        std::cerr << "ERROR\n";
        return 1;
    }
}