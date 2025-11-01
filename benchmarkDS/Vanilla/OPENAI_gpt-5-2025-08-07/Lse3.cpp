#include <iostream>
#include <cstddef>

void* copy_bytes(void* dest, const void* src, std::size_t n) {
    unsigned char* d = static_cast<unsigned char*>(dest);
    const unsigned char* s = static_cast<const unsigned char*>(src);
    for (std::size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dest;
}

int main() {
    const char src[] = "Hello";

    char dest1[10] = {};
    copy_bytes(dest1, src, 5);
    dest1[5] = '\0';
    std::cout << "Test1: " << dest1 << "\n";

    char dest2[5] = {'x','x','x','x','x'};
    copy_bytes(dest2, src, 5);
    char out2[6] = {};
    copy_bytes(out2, dest2, 5);
    out2[5] = '\0';
    std::cout << "Test2: " << out2 << "\n";

    char dest3[6] = {};
    copy_bytes(dest3, src, 5);
    dest3[5] = '\0';
    std::cout << "Test3: " << dest3 << "\n";

    char dest4[100] = {};
    copy_bytes(dest4, src, 5);
    dest4[5] = '\0';
    std::cout << "Test4: " << dest4 << "\n";

    char dest5[5];
    copy_bytes(dest5, src, 5);
    char out5[6] = {};
    copy_bytes(out5, dest5, 5);
    out5[5] = '\0';
    std::cout << "Test5: " << out5 << "\n";

    return 0;
}