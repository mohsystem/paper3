#include <iostream>
#include <cstring>

// Copy n bytes from src to dest. Returns dest on success, nullptr on error.
void* copy_bytes(void* dest, const void* src, size_t n) {
    if (n == 0) return dest;
    if (dest == nullptr || src == nullptr) return nullptr;
    // Use memmove to safely handle potential overlaps
    return std::memmove(dest, src, n);
}

int main() {
    // Test 1: Normal copy of "Hello"
    const char src1[] = "Hello";
    char dest1[10] = {0};
    void* r1p = copy_bytes(dest1, src1, 5);
    dest1[5] = '\0';
    std::cout << "Test1 result=" << (r1p ? 5 : -1) << " dest='" << dest1 << "'\n";

    // Test 2: Copy 0 bytes (no-op)
    char dest2[3] = {0};
    void* r2p = copy_bytes(dest2, src1, 0);
    std::cout << "Test2 result=" << (r2p ? 0 : -1) << "\n";

    // Test 3: Exact size buffer for "Hello"
    char dest3[6] = {0};
    void* r3p = copy_bytes(dest3, src1, 5);
    dest3[5] = '\0';
    std::cout << "Test3 result=" << (r3p ? 5 : -1) << " dest='" << dest3 << "'\n";

    // Test 4: Overlap scenario within the same buffer
    char buf[16] = "HelloWorld";
    void* r4p = copy_bytes(buf + 5, buf, 5); // copy "Hello" to position 5-9
    buf[10] = '\0';
    std::cout << "Test4 result=" << (r4p ? 5 : -1) << " buf='" << buf << "'\n";

    // Test 5: Null source (should fail, return nullptr)
    char dest5[10] = {0};
    void* r5p = copy_bytes(dest5, nullptr, 5);
    std::cout << "Test5 result=" << (r5p ? 5 : -1) << "\n";

    return 0;
}