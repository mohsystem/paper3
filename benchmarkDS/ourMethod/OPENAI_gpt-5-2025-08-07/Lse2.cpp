#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Function: copies a number of bytes from src to dest.
// First argument: destination pointer
// Second argument: source pointer
// Third argument: number of bytes to copy
// Additional arguments: destination capacity and source length for bounds checking.
// Returns: number of bytes actually copied.
std::size_t safe_copy_bytes(std::uint8_t* dest,
                            const std::uint8_t* src,
                            std::size_t n,
                            std::size_t dest_capacity,
                            std::size_t src_length) {
    if (dest == nullptr || src == nullptr || n == 0) {
        return 0;
    }
    std::size_t to_copy = n;
    if (to_copy > dest_capacity) to_copy = dest_capacity;
    if (to_copy > src_length) to_copy = src_length;
    if (to_copy == 0) {
        return 0;
    }
    std::memmove(dest, src, to_copy);
    return to_copy;
}

static void print_result(const std::string& label,
                         const std::vector<std::uint8_t>& buf,
                         std::size_t copied) {
    std::string content(reinterpret_cast<const char*>(buf.data()), copied);
    std::cout << label << " bytes_copied=" << copied << ", content='" << content << "'\n";
}

int main() {
    // Test 1: Copy full message
    const std::string msg1 = "Hello, Secure World!";
    std::vector<std::uint8_t> src1(msg1.begin(), msg1.end());
    std::vector<std::uint8_t> buf1(64, 0);
    std::size_t c1 = safe_copy_bytes(buf1.data(), src1.data(), src1.size(), buf1.size(), src1.size());
    print_result("Test 1", buf1, c1);

    // Test 2: Partial copy
    const std::string msg2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::vector<std::uint8_t> src2(msg2.begin(), msg2.end());
    std::vector<std::uint8_t> buf2(32, 0);
    std::size_t c2 = safe_copy_bytes(buf2.data(), src2.data(), 5, buf2.size(), src2.size());
    print_result("Test 2", buf2, c2);

    // Test 3: n larger than destination capacity
    const std::string msg3 = "1234567890abcdefghijklmnop";
    std::vector<std::uint8_t> src3(msg3.begin(), msg3.end());
    std::vector<std::uint8_t> buf3(10, 0);
    std::size_t c3 = safe_copy_bytes(buf3.data(), src3.data(), 50, buf3.size(), src3.size());
    print_result("Test 3", buf3, c3);

    // Test 4: Zero-length copy
    const std::string msg4 = "NoCopy";
    std::vector<std::uint8_t> src4(msg4.begin(), msg4.end());
    std::vector<std::uint8_t> buf4(10, 0);
    std::size_t c4 = safe_copy_bytes(buf4.data(), src4.data(), 0, buf4.size(), src4.size());
    print_result("Test 4", buf4, c4);

    // Test 5: Empty source
    std::vector<std::uint8_t> src5; // empty
    std::vector<std::uint8_t> buf5(10, 0);
    std::size_t c5 = safe_copy_bytes(buf5.data(), src5.data(), 5, buf5.size(), src5.size());
    print_result("Test 5", buf5, c5);

    return 0;
}