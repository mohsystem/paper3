#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <string>

std::size_t copy_bytes(unsigned char* dest, const unsigned char* src, std::size_t n) {
    if (!dest || !src || n == 0) {
        return 0;
    }
    if (dest == src) {
        return n;
    }
    if (dest < src || dest >= src + n) {
        for (std::size_t i = 0; i < n; ++i) {
            dest[i] = src[i];
        }
    } else {
        for (std::size_t i = n; i > 0; --i) {
            dest[i - 1] = src[i - 1];
        }
    }
    return n;
}

int main() {
    // Test 1: Normal copy
    std::string msg1 = "Secure copy test";
    std::vector<unsigned char> dest1(64, 0);
    std::size_t n1 = msg1.size();
    std::size_t c1 = copy_bytes(dest1.data(), reinterpret_cast<const unsigned char*>(msg1.data()), n1);
    std::cout.write(reinterpret_cast<const char*>(dest1.data()), static_cast<std::streamsize>(c1));
    std::cout << "\n";

    // Test 2: n larger than destination length
    std::string msg2 = "ShortMessage";
    std::vector<unsigned char> dest2(5, 0);
    std::size_t n2 = std::min<std::size_t>({100, dest2.size(), msg2.size()});
    std::size_t c2 = copy_bytes(dest2.data(), reinterpret_cast<const unsigned char*>(msg2.data()), n2);
    std::cout.write(reinterpret_cast<const char*>(dest2.data()), static_cast<std::streamsize>(c2));
    std::cout << "\n";

    // Test 3: n is zero
    std::string msg3 = "ZeroCopy";
    std::vector<unsigned char> dest3(16, 0);
    std::size_t c3 = copy_bytes(dest3.data(), reinterpret_cast<const unsigned char*>(msg3.data()), 0);
    std::cout << "Bytes copied: " << c3 << "\n";

    // Test 4: n larger than source length
    std::string msg4 = "SrcSmall";
    std::vector<unsigned char> dest4(100, 0);
    std::size_t n4 = std::min<std::size_t>(1000, msg4.size());
    std::size_t c4 = copy_bytes(dest4.data(), reinterpret_cast<const unsigned char*>(msg4.data()), n4);
    std::cout.write(reinterpret_cast<const char*>(dest4.data()), static_cast<std::streamsize>(c4));
    std::cout << "\n";

    // Test 5: Overlapping copy within same buffer
    const char* s = "OverlapExample";
    std::vector<unsigned char> buf(64, 0);
    std::memcpy(buf.data(), s, std::strlen(s));
    std::size_t len = std::strlen(s);
    std::size_t safe_n = (len > 2) ? (len - 2) : 0;
    std::size_t c5 = copy_bytes(buf.data() + 2, buf.data(), safe_n);
    std::cout.write(reinterpret_cast<const char*>(buf.data()), static_cast<std::streamsize>(safe_n + 2));
    std::cout << "\n";

    return 0;
}