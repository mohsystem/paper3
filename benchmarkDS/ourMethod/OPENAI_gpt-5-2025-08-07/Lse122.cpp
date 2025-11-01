#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

std::array<uint8_t, 10> allocateCopyFree(const std::vector<uint8_t>& input) {
    constexpr std::size_t SIZE = 10;

    // Allocate and zero-initialize first chunk
    std::unique_ptr<uint8_t[]> chunk1(new (std::nothrow) uint8_t[SIZE]());
    if (!chunk1) {
        throw std::bad_alloc();
    }

    // Copy bounded input into chunk1
    const std::size_t copyLen = std::min(SIZE, input.size());
    for (std::size_t i = 0; i < copyLen; ++i) {
        chunk1[i] = input[i];
    }

    // Allocate second chunk
    std::unique_ptr<uint8_t[]> chunk2(new (std::nothrow) uint8_t[SIZE]());
    if (!chunk2) {
        // zeroize and free chunk1 before rethrow
        std::fill_n(chunk1.get(), SIZE, 0);
        throw std::bad_alloc();
    }

    // Copy chunk1 -> chunk2 (bounded)
    std::copy_n(chunk1.get(), SIZE, chunk2.get());

    // Detach the result before freeing both chunks
    std::array<uint8_t, SIZE> result{};
    for (std::size_t i = 0; i < SIZE; ++i) {
        result[i] = chunk2[i];
    }

    // Zeroize then allow RAII to free memory
    std::fill_n(chunk1.get(), SIZE, 0);
    std::fill_n(chunk2.get(), SIZE, 0);

    return result;
}

static void printArray(const std::array<uint8_t, 10>& arr) {
    std::cout << "[";
    for (std::size_t i = 0; i < arr.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << static_cast<unsigned>(arr[i]);
    }
    std::cout << "]\n";
}

int main() {
    // 5 test cases
    std::array<uint8_t, 10> r1 = allocateCopyFree(std::vector<uint8_t>{1, 2, 3});
    std::array<uint8_t, 10> r2 = allocateCopyFree(std::vector<uint8_t>{});
    std::array<uint8_t, 10> r3 = allocateCopyFree(std::vector<uint8_t>{0,1,2,3,4,5,6,7,8,9});
    std::array<uint8_t, 10> r4 = allocateCopyFree(std::vector<uint8_t>{10,11,12,13,14,15,16,17,18,19,20}); // longer than 10
    std::array<uint8_t, 10> r5 = allocateCopyFree(std::vector<uint8_t>{255, 0, 128});

    printArray(r1);
    printArray(r2);
    printArray(r3);
    printArray(r4);
    printArray(r5);

    return 0;
}