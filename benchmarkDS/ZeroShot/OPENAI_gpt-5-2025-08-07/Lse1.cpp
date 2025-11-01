#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iomanip>

void* memmove_secure(void* dest, const void* src, std::size_t count) {
    if (dest == nullptr || src == nullptr) {
        return nullptr;
    }
    if (count == 0 || dest == src) {
        return dest;
    }
    auto* d = static_cast<std::uint8_t*>(dest);
    const auto* s = static_cast<const std::uint8_t*>(src);

    if (d < s) {
        for (std::size_t i = 0; i < count; ++i) {
            d[i] = s[i];
        }
    } else {
        for (std::size_t i = count; i > 0; --i) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

static void print_hex(const std::uint8_t* buf, std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                  << static_cast<int>(buf[i]) << (i + 1 < len ? " " : "");
    }
    std::cout << std::dec;
}

int main() {
    // Test 1: Copy 10 bytes "HelloWorld"
    const char src1[] = "HelloWorld";
    char dest1[11] = {0};
    memmove_secure(dest1, src1, 10);
    std::cout << "Test1: " << dest1 << "\n";

    // Test 2: Copy 3 bytes from "abcdef"
    const char src2[] = "abcdef";
    std::uint8_t dest2[6] = {0};
    memmove_secure(dest2, src2, 3);
    std::cout << "Test2: ";
    std::cout.write(reinterpret_cast<char*>(dest2), 6);
    std::cout << " | Hex: ";
    print_hex(dest2, 6);
    std::cout << "\n";

    // Test 3: Zero bytes copy (no change)
    const char src3[] = "xyz";
    std::uint8_t dest3[3] = {0};
    memmove_secure(dest3, src3, 0);
    std::cout << "Test3: ";
    print_hex(dest3, 3);
    std::cout << "\n";

    // Test 4: Null arguments (should return nullptr)
    void* res4 = memmove_secure(nullptr, src1, 5);
    std::cout << "Test4: " << (res4 == nullptr ? "nullptr" : "not null") << "\n";

    // Test 5: Same buffer as both src and dest
    char both[] = "12345";
    memmove_secure(both, both, 4);
    std::cout << "Test5: " << both << "\n";

    return 0;
}