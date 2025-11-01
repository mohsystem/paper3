#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// Step 1-5: Secure byte copy with validation and overlap-safety using a temporary buffer.
size_t copyBytes(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src, size_t n) {
    if (n == 0 || dest.empty() || src.empty()) return 0;
    size_t bytesToCopy = std::min(n, std::min(dest.size(), src.size()));
    if (bytesToCopy == 0) return 0;
    std::vector<uint8_t> temp(src.begin(), src.begin() + bytesToCopy); // overlap-safe
    std::copy(temp.begin(), temp.end(), dest.begin());
    return bytesToCopy;
}
// End of secure copy

static std::string toStringBytes(const std::vector<uint8_t>& v, size_t count) {
    size_t c = std::min(count, v.size());
    return std::string(v.begin(), v.begin() + c);
}

int main() {
    // Test 1: Copy "Hello" (5 bytes)
    std::vector<uint8_t> dest1(16);
    std::vector<uint8_t> src1 = {'H','e','l','l','o'};
    size_t c1 = copyBytes(dest1, src1, 5);
    std::cout << "Test1 bytesCopied=" << c1 << " content=\"" << toStringBytes(dest1, c1) << "\"\n";

    // Test 2: Request more than source length (10) - clamp to 5
    std::vector<uint8_t> dest2(16);
    size_t c2 = copyBytes(dest2, src1, 10);
    std::cout << "Test2 bytesCopied=" << c2 << " content=\"" << toStringBytes(dest2, c2) << "\"\n";

    // Test 3: Destination smaller (3)
    std::vector<uint8_t> dest3(3);
    size_t c3 = copyBytes(dest3, src1, 5);
    std::cout << "Test3 bytesCopied=" << c3 << " content=\"" << toStringBytes(dest3, c3) << "\"\n";

    // Test 4: Zero length copy
    std::vector<uint8_t> dest4(10);
    size_t c4 = copyBytes(dest4, src1, 0);
    std::cout << "Test4 bytesCopied=" << c4 << " content=\"" << toStringBytes(dest4, c4) << "\"\n";

    // Test 5: Empty source
    std::vector<uint8_t> dest5(10);
    std::vector<uint8_t> src5;
    size_t c5 = copyBytes(dest5, src5, 5);
    std::cout << "Test5 bytesCopied=" << c5 << " content=\"" << toStringBytes(dest5, c5) << "\"\n";

    return 0;
}