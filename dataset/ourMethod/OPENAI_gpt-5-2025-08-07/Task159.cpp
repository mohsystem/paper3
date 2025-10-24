#include <iostream>
#include <vector>
#include <cstdint>
#include <stdexcept>

struct ReadResult {
    bool success;
    uint32_t value;
};

static const size_t MAX_BUFFER_SIZE = 1048576;

std::vector<uint8_t> allocateBuffer(size_t size) {
    if (size > MAX_BUFFER_SIZE) {
        throw std::invalid_argument("Invalid buffer size");
    }
    return std::vector<uint8_t>(size);
}

void fillSequential(std::vector<uint8_t>& buffer) {
    for (size_t i = 0; i < buffer.size(); ++i) {
        buffer[i] = static_cast<uint8_t>(i & 0xFF);
    }
}

ReadResult safeRead(const std::vector<uint8_t>& buffer, int index) {
    if (index < 0) {
        return {false, 0};
    }
    size_t uidx = static_cast<size_t>(index);
    if (uidx >= buffer.size()) {
        return {false, 0};
    }
    return {true, static_cast<uint32_t>(buffer[uidx])};
}

int main() {
    std::vector<uint8_t> buffer = allocateBuffer(10);
    fillSequential(buffer);

    int testIndices[5] = { -1, 0, 5, 9, 10 };
    for (int idx : testIndices) {
        ReadResult res = safeRead(buffer, idx);
        if (res.success) {
            std::cout << "Index " << idx << " -> value: " << res.value << "\n";
        } else {
            std::cout << "Index " << idx << " -> error: out of bounds\n";
        }
    }

    std::cout << "Buffer length: " << buffer.size() << "\n";
    std::cout << "Buffer content: [";
    for (size_t i = 0; i < buffer.size(); ++i) {
        std::cout << static_cast<uint32_t>(buffer[i]);
        if (i + 1 < buffer.size()) std::cout << ", ";
    }
    std::cout << "]\n";

    return 0;
}