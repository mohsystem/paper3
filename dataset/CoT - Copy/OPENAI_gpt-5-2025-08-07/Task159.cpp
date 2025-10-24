// Chain-of-Through process in code generation applied with secure checks.
// Language: C++

#include <iostream>
#include <vector>
#include <cstdint>
#include <limits>

struct ReadResult {
    bool success;
    uint8_t value;
};

static const size_t MAX_ALLOC = 10'000'000; // Prevent excessive allocation

std::vector<uint8_t> allocate_buffer(size_t size) {
    if (size > MAX_ALLOC) {
        throw std::invalid_argument("Invalid buffer size");
    }
    std::vector<uint8_t> buf(size);
    for (size_t i = 0; i < size; ++i) {
        buf[i] = static_cast<uint8_t>(i & 0xFF);
    }
    return buf;
}

ReadResult read_from_buffer(const std::vector<uint8_t>& buffer, long long index) {
    if (index < 0) {
        return {false, 0};
    }
    size_t idx = static_cast<size_t>(index);
    if (idx >= buffer.size()) {
        return {false, 0};
    }
    return {true, buffer[idx]};
}

int main() {
    auto buffer = allocate_buffer(10);
    long long test_indices[5] = {0, 5, 9, -1, 10};

    for (long long idx : test_indices) {
        ReadResult res = read_from_buffer(buffer, idx);
        if (res.success) {
            std::cout << "Index " << idx << " -> " << static_cast<unsigned int>(res.value) << "\n";
        } else {
            std::cout << "Index " << idx << " -> invalid index\n";
        }
    }
    return 0;
}