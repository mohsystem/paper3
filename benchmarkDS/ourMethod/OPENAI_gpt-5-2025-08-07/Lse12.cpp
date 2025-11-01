#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

static int getValueFromArray(const std::vector<int>& arr, std::size_t size, long long index) {
    // Returns the value at index if valid; otherwise returns 1 (error code).
    if (size > arr.size()) return 1;
    if (index < 0) return 1;
    if (static_cast<unsigned long long>(index) >= static_cast<unsigned long long>(size)) return 1;
    return arr[static_cast<std::size_t>(index)];
}

static void runTests() {
    std::vector<int> data{10, 20, 30, 40, 50};
    assert(getValueFromArray(data, data.size(), 0) == 10);
    assert(getValueFromArray(data, data.size(), 1) == 20);
    assert(getValueFromArray(data, data.size(), 2) == 30);
    assert(getValueFromArray(data, data.size(), 4) == 50);
    assert(getValueFromArray(data, data.size(), 5) == 1); // out-of-bounds -> 1
}

int main() {
    runTests();

    std::vector<int> data{10, 20, 30, 40, 50};
    long long idx = -1; // invalid by default

    std::string line;
    if (std::getline(std::cin, line)) {
        try {
            std::size_t pos = 0;
            long long val = std::stoll(line, &pos, 10);
            // ensure no trailing non-space characters
            while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos]))) ++pos;
            if (pos == line.size()) {
                idx = val;
            } else {
                idx = -1;
            }
        } catch (...) {
            idx = -1;
        }
    }

    int result = getValueFromArray(data, data.size(), idx);
    return result & 0xFF;
}