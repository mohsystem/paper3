// Chain-of-Through Process:
// 1) Problem understanding: Generate secure random numbers within a range and secure tokens.
// 2) Security requirements: Use OS-provided CSPRNG (BCryptGenRandom on Windows; getrandom or /dev/urandom on POSIX).
// 3) Secure coding generation: Validate inputs, avoid modulo bias via rejection sampling, handle errors robustly.
// 4) Code review: Check for integer overflows, resource handling, and platform differences.
// 5) Secure code output: Final code implements secure bytes function, base64url encoding without padding, and test cases.

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <limits>
#include <cstring>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #ifdef __linux__
    #include <sys/random.h>
  #endif
  #include <errno.h>
#endif

static bool secure_random_bytes(unsigned char* buffer, size_t len) {
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buffer, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
  #ifdef __linux__
    size_t total = 0;
    while (total < len) {
        ssize_t r = getrandom(buffer + total, len - total, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        total += static_cast<size_t>(r);
    }
    if (total == len) return true;
    // Fallback to /dev/urandom if getrandom fails
  #endif
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, buffer + total, len - total);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (r == 0) {
            close(fd);
            return false;
        }
        total += static_cast<size_t>(r);
    }
    close(fd);
    return true;
#endif
}

static int secure_random_in_range(int minVal, int maxVal) {
    if (minVal > maxVal) {
        throw std::invalid_argument("minVal must be <= maxVal");
    }
    uint64_t range = (uint64_t)((int64_t)maxVal - (int64_t)minVal) + 1ULL; // range in [1, 2^64]
    // Rejection sampling on 64-bit
    uint64_t limit = std::numeric_limits<uint64_t>::max() - (std::numeric_limits<uint64_t>::max() % range);
    while (true) {
        uint64_t x = 0;
        if (!secure_random_bytes(reinterpret_cast<unsigned char*>(&x), sizeof(x))) {
            throw std::runtime_error("secure_random_bytes failed");
        }
        if (x < limit) {
            return minVal + static_cast<int>(x % range);
        }
    }
}

static std::vector<int> generate_random_numbers(size_t count, int minVal, int maxVal) {
    if (minVal > maxVal) {
        throw std::invalid_argument("minVal must be <= maxVal");
    }
    std::vector<int> out;
    out.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        out.push_back(secure_random_in_range(minVal, maxVal));
    }
    return out;
}

// Base64URL encoding without padding
static std::string base64url_encode(const unsigned char* data, size_t len) {
    static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t full_groups = len / 3;
    size_t rem = len % 3;
    size_t out_len = full_groups * 4 + (rem ? (rem + 1) : 0); // no padding
    std::string out;
    out.reserve(out_len);

    size_t i = 0;
    for (size_t g = 0; g < full_groups; ++g) {
        unsigned int b0 = data[i++];
        unsigned int b1 = data[i++];
        unsigned int b2 = data[i++];
        out.push_back(alphabet[(b0 >> 2) & 0x3F]);
        out.push_back(alphabet[((b0 & 0x3) << 4) | ((b1 >> 4) & 0x0F)]);
        out.push_back(alphabet[((b1 & 0x0F) << 2) | ((b2 >> 6) & 0x03)]);
        out.push_back(alphabet[b2 & 0x3F]);
    }

    if (rem == 1) {
        unsigned int b0 = data[i++];
        out.push_back(alphabet[(b0 >> 2) & 0x3F]);
        out.push_back(alphabet[(b0 & 0x3) << 4]);
    } else if (rem == 2) {
        unsigned int b0 = data[i++];
        unsigned int b1 = data[i++];
        out.push_back(alphabet[(b0 >> 2) & 0x3F]);
        out.push_back(alphabet[((b0 & 0x3) << 4) | ((b1 >> 4) & 0x0F)]);
        out.push_back(alphabet[(b1 & 0x0F) << 2]);
    }
    return out;
}

static std::string generate_secure_token(size_t byteLength) {
    if (byteLength == 0) {
        throw std::invalid_argument("byteLength must be positive");
    }
    std::vector<unsigned char> buf(byteLength);
    if (!secure_random_bytes(buf.data(), buf.size())) {
        throw std::runtime_error("secure_random_bytes failed");
    }
    return base64url_encode(buf.data(), buf.size());
}

static void print_vec(const std::vector<int>& v) {
    std::cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

int main() {
    try {
        // 1) 5 numbers between 1 and 10
        std::cout << "Test1 numbers [1,10]:" << std::endl;
        auto a = generate_random_numbers(5, 1, 10);
        print_vec(a);

        // 2) Secure token with 16 bytes
        std::cout << "Test2 token (16 bytes):" << std::endl;
        std::cout << generate_secure_token(16) << std::endl;

        // 3) 10 numbers between -50 and 50
        std::cout << "Test3 numbers [-50,50]:" << std::endl;
        auto b = generate_random_numbers(10, -50, 50);
        print_vec(b);

        // 4) Secure token with 32 bytes
        std::cout << "Test4 token (32 bytes):" << std::endl;
        std::cout << generate_secure_token(32) << std::endl;

        // 5) 3 numbers in [0,0]
        std::cout << "Test5 numbers [0,0]:" << std::endl;
        auto c = generate_random_numbers(3, 0, 0);
        print_vec(c);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}