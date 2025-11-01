// 1) Problem understanding: copy N bytes from src to dest.
// 2) Security requirements: handle null pointers; support overlap safely.
// 3) Secure coding generation: implement memmove-like logic.
// 4) Code review: bounds cannot be inferred; rely on caller; tests allocate adequate buffers.
// 5) Secure code output: final function below.

#include <iostream>
#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>

void* copy_bytes(void* destination, const void* source, std::size_t n) {
    if (!destination || !source) return destination;
    if (n == 0 || destination == source) return destination;

    unsigned char* dest = static_cast<unsigned char*>(destination);
    const unsigned char* src = static_cast<const unsigned char*>(source);

    if (dest < src || dest >= src + n) {
        for (std::size_t i = 0; i < n; ++i) dest[i] = src[i];
    } else {
        for (std::size_t i = n; i > 0; --i) dest[i - 1] = src[i - 1];
    }
    return destination;
}

static void print_result(const std::string& label, const std::vector<unsigned char>& buf, std::size_t copied) {
    std::string s(buf.begin(), buf.begin() + static_cast<std::ptrdiff_t>(copied));
    std::cout << label << " | copied=" << copied << " | result=\"" << s << "\"\n";
}

int main() {
    std::string msg = "Hello, World!";
    std::vector<unsigned char> src(msg.begin(), msg.end());

    // Test 1: exact-size buffer
    std::vector<unsigned char> dest1(src.size());
    copy_bytes(dest1.data(), src.data(), src.size());
    print_result("Test1 (exact copy)", dest1, src.size());

    // Test 2: larger buffer
    std::vector<unsigned char> dest2(64);
    copy_bytes(dest2.data(), src.data(), src.size());
    print_result("Test2 (into larger buffer)", dest2, src.size());

    // Test 3: partial copy (first 5 bytes)
    std::vector<unsigned char> dest3(5);
    copy_bytes(dest3.data(), src.data(), 5);
    print_result("Test3 (partial 5 bytes)", dest3, 5);

    // Test 4: zero-length copy
    std::vector<unsigned char> dest4(10);
    copy_bytes(dest4.data(), src.data(), 0);
    print_result("Test4 (zero bytes)", dest4, 0);

    // Test 5: overlapping copy within same buffer (safe)
    std::vector<unsigned char> buf(msg.begin(), msg.end()); // "Hello, World!"
    // Shift first 5 bytes by 2 positions to the right
    copy_bytes(buf.data() + 2, buf.data(), 5);
    print_result("Test5 (overlap shift right by 2 of first 5 bytes)", buf, buf.size());

    return 0;
}