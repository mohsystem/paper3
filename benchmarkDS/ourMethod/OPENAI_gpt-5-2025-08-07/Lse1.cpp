#include <iostream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <limits>

int mem_copy(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src, int n) {
    if (n < 0) return -1;
    if (static_cast<long long>(n) > std::numeric_limits<int>::max()) return -1;
    if (static_cast<size_t>(n) > dest.size() || static_cast<size_t>(n) > src.size()) return -1;
    if (n == 0) return 0;
    // Use memmove semantics to be safe even if dest and src refer to same underlying buffer
    std::memmove(dest.data(), src.data(), static_cast<size_t>(n));
    return n;
}

std::string to_hex(const std::vector<uint8_t>& v) {
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i) {
        oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)v[i];
        if (i + 1 < v.size()) oss << ' ';
    }
    return oss.str();
}

int main() {
    // Test 1: normal copy of 5 bytes
    std::vector<uint8_t> src1(10);
    for (size_t i = 0; i < src1.size(); ++i) src1[i] = static_cast<uint8_t>(i);
    std::vector<uint8_t> dest1(10, 0);
    int r1 = mem_copy(dest1, src1, 5);
    std::cout << "Test1 result=" << r1 << " dest1=" << to_hex(dest1) << "\n";

    // Test 2: copy zero bytes
    std::vector<uint8_t> src2{1,2,3,4,5};
    std::vector<uint8_t> dest2{9,9,9,9,9};
    int r2 = mem_copy(dest2, src2, 0);
    std::cout << "Test2 result=" << r2 << " dest2=" << to_hex(dest2) << "\n";

    // Test 3: copy full size exact
    std::vector<uint8_t> src3{'A','B','C','D','E','F','G','H'};
    std::vector<uint8_t> dest3(8, 0);
    int r3 = mem_copy(dest3, src3, 8);
    std::cout << "Test3 result=" << r3 << " dest3=" << to_hex(dest3) << "\n";

    // Test 4: attempt to copy too many bytes (should fail)
    std::vector<uint8_t> src4{10,20,30,40,50};
    std::vector<uint8_t> dest4{0,0,0};
    int r4 = mem_copy(dest4, src4, 4);
    std::cout << "Test4 result=" << r4 << " dest4=" << to_hex(dest4) << "\n";

    // Test 5: same vector as both src and dest
    std::vector<uint8_t> same{7,7,7,7,7,7};
    int r5 = mem_copy(same, same, 6);
    std::cout << "Test5 result=" << r5 << " same=" << to_hex(same) << "\n";

    return 0;
}