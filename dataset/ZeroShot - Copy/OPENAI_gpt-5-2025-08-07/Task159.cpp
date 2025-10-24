#include <iostream>
#include <vector>
#include <limits>

static const size_t MAX_SIZE = 1000000;

std::vector<int> allocate_buffer(size_t size, int startVal) {
    if (size > MAX_SIZE) {
        return {};
    }
    std::vector<int> buffer;
    try {
        buffer.resize(size);
    } catch (...) {
        return {};
    }
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = startVal + static_cast<int>(i);
    }
    return buffer;
}

bool read_at(const std::vector<int>& buffer, size_t index, int& outVal) {
    if (index >= buffer.size()) {
        return false;
    }
    outVal = buffer[index];
    return true;
}

int main() {
    // 5 test cases
    auto buf1 = allocate_buffer(10, 100);
    int val = 0;
    std::cout << "Test1: " << (read_at(buf1, 0, val) ? std::to_string(val) : "Invalid") << "\n";

    auto buf2 = allocate_buffer(10, 200);
    std::cout << "Test2: " << (read_at(buf2, 9, val) ? std::to_string(val) : "Invalid") << "\n";

    auto buf3 = allocate_buffer(10, 300);
    std::cout << "Test3: " << (read_at(buf3, 10, val) ? std::to_string(val) : "Invalid") << "\n";

    auto buf4 = allocate_buffer(0, 400);
    std::cout << "Test4: " << (read_at(buf4, 0, val) ? std::to_string(val) : "Invalid") << "\n";

    auto buf5 = allocate_buffer(5, 500);
    std::cout << "Test5: " << (read_at(buf5, static_cast<size_t>(-1), val) ? std::to_string(val) : "Invalid") << "\n";

    return 0;
}