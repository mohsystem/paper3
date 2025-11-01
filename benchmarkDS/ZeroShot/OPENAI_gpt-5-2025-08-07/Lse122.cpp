#include <array>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <new>

std::array<uint8_t, 10> copyTwoChunks(const std::array<uint8_t, 10>& src) {
    std::array<uint8_t, 10> out{}; // default-initialized to zeros

    uint8_t* chunk1 = new (std::nothrow) uint8_t[10];
    uint8_t* chunk2 = new (std::nothrow) uint8_t[10];

    if (!chunk1 || !chunk2) {
        delete[] chunk1;
        delete[] chunk2;
        return out;
    }

    std::copy_n(src.begin(), 10, chunk1);
    std::copy_n(chunk1, 10, chunk2);
    std::copy_n(chunk2, 10, out.begin());

    // Securely clear before free (best-effort)
    std::fill(chunk1, chunk1 + 10, 0);
    std::fill(chunk2, chunk2 + 10, 0);

    delete[] chunk1;
    delete[] chunk2;

    return out;
}

static void printArray(const std::array<uint8_t,10>& a) {
    std::cout << "[";
    for (size_t i = 0; i < a.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << static_cast<unsigned int>(a[i]);
    }
    std::cout << "]\n";
}

int main() {
    std::array<uint8_t,10> tests[5] = {
        std::array<uint8_t,10>{0,1,2,3,4,5,6,7,8,9},
        std::array<uint8_t,10>{10,20,30,40,50,60,70,80,90,100},
        std::array<uint8_t,10>{255,254,253,252,251,250,249,248,247,246},
        std::array<uint8_t,10>{42,42,42,42,42,42,42,42,42,42},
        std::array<uint8_t,10>{9,8,7,6,5,4,3,2,1,0}
    };

    for (const auto& t : tests) {
        auto res = copyTwoChunks(t);
        printArray(res);
    }
    return 0;
}