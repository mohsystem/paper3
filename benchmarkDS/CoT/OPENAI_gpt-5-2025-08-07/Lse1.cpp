#include <iostream>
#include <cstring>
#include <vector>

void* safe_memcpy(void* dest, const void* src, size_t n) {
    if (n == 0) return dest;
    if (dest == nullptr || src == nullptr) return nullptr;
    if (dest == src) return dest;

    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);

    // Handle overlap safely (memmove semantics)
    if (d < s || d >= s + n) {
        // No harmful overlap or dest is before src: copy forward
        for (size_t i = 0; i < n; ++i) d[i] = s[i];
    } else {
        // Overlap with dest after src: copy backward
        for (size_t i = n; i > 0; --i) d[i - 1] = s[i - 1];
    }
    return dest;
}

static void print_bytes(const char* label, const unsigned char* data, size_t n) {
    std::cout << label << ": [";
    for (size_t i = 0; i < n; ++i) {
        std::cout << static_cast<int>(data[i]);
        if (i + 1 < n) std::cout << ", ";
    }
    std::cout << "]\n";
}

int main() {
    // Test 1: Basic copy
    unsigned char src1[] = {1, 2, 3, 4, 5};
    unsigned char dst1[5] = {0};
    safe_memcpy(dst1, src1, 5);
    print_bytes("Test1 dest", dst1, 5);

    // Test 2: Zero-length copy
    unsigned char src2[] = {7, 8, 9};
    unsigned char dst2[] = {9, 9, 9};
    safe_memcpy(dst2, src2, 0);
    print_bytes("Test2 dest", dst2, 3);

    // Test 3: Overlapping forward copy (dest starts inside src)
    unsigned char buf3[] = {'a','b','c','d','e','f','g','\0'};
    safe_memcpy(buf3 + 2, buf3, 5); // copy "abcde" to start at index 2
    std::cout << "Test3 dest: " << buf3 << "\n";

    // Test 4: Overlapping backward copy (src starts inside dest)
    unsigned char buf4[] = {'1','2','3','4','5','6','7','\0'};
    safe_memcpy(buf4, buf4 + 2, 5); // copy "34567" to start at index 0
    std::cout << "Test4 dest: " << buf4 << "\n";

    // Test 5: Copy using vector buffers
    std::vector<unsigned char> vsrc = {10, 20, 30, 40, 50};
    std::vector<unsigned char> vdst(5, 0);
    safe_memcpy(vdst.data(), vsrc.data(), vsrc.size());
    print_bytes("Test5 dest", vdst.data(), vdst.size());

    return 0;
}