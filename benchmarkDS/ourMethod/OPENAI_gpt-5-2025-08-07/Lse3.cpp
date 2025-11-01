#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>

std::size_t copy_bytes(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src, std::size_t n) {
    std::size_t m = std::min({n, dest.size(), src.size()});
    if (m > 0) {
        std::memmove(dest.data(), src.data(), m);
    }
    return m;
}

int main() {
    const std::string s = "Hello";
    const std::vector<uint8_t> hello(s.begin(), s.end());

    // Test 1: Exact fit
    std::vector<uint8_t> dest1(5, 0);
    std::size_t copied1 = copy_bytes(dest1, hello, 5);
    std::cout << "Test1 copied: " << copied1 << " data: " << std::string(dest1.begin(), dest1.begin() + copied1) << "\n";

    // Test 2: Destination smaller than requested
    std::vector<uint8_t> dest2(3, 0);
    std::size_t copied2 = copy_bytes(dest2, hello, 5);
    std::cout << "Test2 copied: " << copied2 << " data: " << std::string(dest2.begin(), dest2.begin() + copied2) << "\n";

    // Test 3: Destination larger than requested (exact request)
    std::vector<uint8_t> dest3(10, 0);
    std::size_t copied3 = copy_bytes(dest3, hello, 5);
    std::cout << "Test3 copied: " << copied3 << " data: " << std::string(dest3.begin(), dest3.begin() + copied3) << "\n";

    // Test 4: Copy fewer bytes than available
    std::vector<uint8_t> dest4(10, 0);
    std::size_t copied4 = copy_bytes(dest4, hello, 3);
    std::cout << "Test4 copied: " << copied4 << " data: " << std::string(dest4.begin(), dest4.begin() + copied4) << "\n";

    // Test 5: Copy zero bytes
    std::vector<uint8_t> dest5(5, 0);
    std::size_t copied5 = copy_bytes(dest5, hello, 0);
    std::cout << "Test5 copied: " << copied5 << " data: " << std::string(dest5.begin(), dest5.begin() + copied5) << "\n";

    return 0;
}