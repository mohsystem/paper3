#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>
#include <stdexcept>

void mem_copy(uint8_t* dest, const uint8_t* src, size_t n) {
    if (!dest || !src) throw std::invalid_argument("Null buffer");
    for (size_t i = 0; i < n; ++i) dest[i] = src[i];
}

void print_hex(const std::vector<uint8_t>& v, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        std::cout << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)v[i];
        if (i + 1 < n) std::cout << " ";
    }
    std::cout << std::dec << "\n";
}

int main() {
    // Test 1: Copy full ASCII buffer
    std::vector<uint8_t> src1 = {'H','e','l','l','o','W','o','r','l','d'};
    std::vector<uint8_t> dest1(10, 0);
    mem_copy(dest1.data(), src1.data(), 10);
    std::cout << "T1 HEX: "; print_hex(dest1, 10);
    std::cout << "T1 STR: " << std::string(dest1.begin(), dest1.end()) << "\n";

    // Test 2: Partial copy
    std::vector<uint8_t> src2 = {'O','p','e','n','A','I'};
    std::vector<uint8_t> dest2(4, 0);
    mem_copy(dest2.data(), src2.data(), 4); // "Open"
    std::cout << "T2 HEX: "; print_hex(dest2, 4);
    std::cout << "T2 STR: " << std::string(dest2.begin(), dest2.end()) << "\n";

    // Test 3: Zero-length copy (dest remains unchanged)
    std::vector<uint8_t> src3 = {'X','Y','Z'};
    std::vector<uint8_t> dest3 = {0xEE, 0xEE, 0xEE};
    mem_copy(dest3.data(), src3.data(), 0);
    std::cout << "T3 HEX: "; print_hex(dest3, dest3.size());

    // Test 4: Binary data copy
    std::vector<uint8_t> src4 = {0x00, 0x01, 0xAB, 0xFF, 0x10};
    std::vector<uint8_t> dest4(5, 0);
    mem_copy(dest4.data(), src4.data(), 5);
    std::cout << "T4 HEX: "; print_hex(dest4, 5);

    // Test 5: Copy first 7 bytes
    std::vector<uint8_t> src5 = {'1','2','3','4','5','6','7','8','9','0'};
    std::vector<uint8_t> dest5(7, 0);
    mem_copy(dest5.data(), src5.data(), 7); // "1234567"
    std::cout << "T5 HEX: "; print_hex(dest5, 7);
    std::cout << "T5 STR: " << std::string(dest5.begin(), dest5.end()) << "\n";

    return 0;
}