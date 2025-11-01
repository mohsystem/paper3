#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>

int do_memory_ops(const std::vector<uint8_t>& data) {
    const size_t SIZE = 10;
    uint8_t* first = new uint8_t[SIZE]();
    uint8_t* second = new uint8_t[SIZE]();

    size_t n = data.size() < SIZE ? data.size() : SIZE;
    if (n > 0) {
        std::memcpy(first, data.data(), n);
    }

    std::memcpy(second, first, SIZE);

    int sum = 0;
    for (size_t i = 0; i < SIZE; ++i) {
        sum += second[i];
    }

    delete[] first;  // free first chunk
    delete[] second; // free second chunk

    return sum;
}

int main() {
    std::vector<uint8_t> t1 = {0,1,2,3,4,5,6,7,8,9};
    std::cout << do_memory_ops(t1) << std::endl; // 45

    std::vector<uint8_t> t2(10, 1);
    std::cout << do_memory_ops(t2) << std::endl; // 10

    std::vector<uint8_t> t3 = {10,11,12,13,14,15,16,17,18,19};
    std::cout << do_memory_ops(t3) << std::endl; // 145

    std::vector<uint8_t> t4(10, 255);
    std::cout << do_memory_ops(t4) << std::endl; // 2550

    std::vector<uint8_t> t5 = {5,4,3,2,1,0,250,251,252,253};
    std::cout << do_memory_ops(t5) << std::endl; // 1021

    return 0;
}