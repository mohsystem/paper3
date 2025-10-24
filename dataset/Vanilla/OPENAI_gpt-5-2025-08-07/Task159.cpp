#include <iostream>
#include <new>

// Dynamically allocates an int buffer, fills it, and returns the value at the given index.
// Returns -1 for invalid size, allocation failure, or out-of-bounds index.
int readFromBuffer(std::size_t size, long long index, int fillBase) {
    if (size == 0) return -1;
    int* buffer = new (std::nothrow) int[size];
    if (!buffer) return -1;
    for (std::size_t i = 0; i < size; ++i) {
        buffer[i] = fillBase + static_cast<int>(i);
    }
    int result = -1;
    if (index >= 0 && static_cast<std::size_t>(index) < size) {
        result = buffer[index];
    }
    delete[] buffer;
    return result;
}

int main() {
    std::size_t size = 5;
    int fillBase = 100;
    long long testIndices[5] = {0, 4, 5, -1, 2};

    for (long long idx : testIndices) {
        int result = readFromBuffer(size, idx, fillBase);
        std::cout << "Index " << idx << " -> " << result << "\n";
    }
    return 0;
}