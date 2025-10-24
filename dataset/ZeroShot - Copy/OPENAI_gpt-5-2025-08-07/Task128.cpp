#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <cstring>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #include <stdlib.h> // arc4random_buf
#elif defined(__linux__)
  #include <unistd.h>
  #include <sys/random.h>
  #include <errno.h>
  #include <fcntl.h>
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
#endif

class Task128 {
public:
    static std::vector<uint8_t> secure_random_bytes(size_t n) {
        std::vector<uint8_t> out(n);
        if (n > 0) {
            os_random_bytes(out.data(), n);
        }
        return out;
    }

    static std::string secure_hex_token(size_t num_bytes) {
        auto bytes = secure_random_bytes(num_bytes);
        static const char* hex = "0123456789abcdef";
        std::string s;
        s.reserve(bytes.size() * 2);
        for (auto b : bytes) {
            s.push_back(hex[(b >> 4) & 0xF]);
            s.push_back(hex[b & 0xF]);
        }
        return s;
    }

    static uint64_t secure_random_uint64(uint64_t min_inclusive, uint64_t max_inclusive) {
        if (min_inclusive > max_inclusive) {
            throw std::invalid_argument("min > max");
        }
        uint64_t range = max_inclusive - min_inclusive + 1;
        if (range == 0) {
            throw std::invalid_argument("Range too large");
        }
        uint64_t limit = UINT64_MAX - (UINT64_MAX % range);
        uint64_t r;
        do {
            r = rand_u64();
        } while (r > limit);
        return min_inclusive + (r % range);
    }

private:
    static uint64_t rand_u64() {
        uint64_t v = 0;
        os_random_bytes(reinterpret_cast<unsigned char*>(&v), sizeof(v));
        return v;
    }

    static void os_random_bytes(unsigned char* buf, size_t n) {
    #if defined(_WIN32)
        if (BCryptGenRandom(nullptr, buf, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
            throw std::runtime_error("BCryptGenRandom failed");
        }
    #elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
        arc4random_buf(buf, n);
    #elif defined(__linux__)
        size_t off = 0;
        while (off < n) {
            ssize_t r = getrandom(buf + off, n - off, 0);
            if (r < 0) {
                if (errno == EINTR) continue;
                if (errno == ENOSYS) break; // fallback to /dev/urandom
                throw std::runtime_error("getrandom failed");
            }
            off += (size_t)r;
        }
        if (off < n) {
            int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
            if (fd < 0) throw std::runtime_error("open /dev/urandom failed");
            while (off < n) {
                ssize_t r = read(fd, buf + off, n - off);
                if (r < 0) {
                    if (errno == EINTR) continue;
                    close(fd);
                    throw std::runtime_error("read /dev/urandom failed");
                }
                if (r == 0) {
                    close(fd);
                    throw std::runtime_error("EOF /dev/urandom");
                }
                off += (size_t)r;
            }
            close(fd);
        }
    #else
        size_t off = 0;
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) throw std::runtime_error("open /dev/urandom failed");
        while (off < n) {
            ssize_t r = read(fd, buf + off, n - off);
            if (r < 0) {
                if (errno == EINTR) continue;
                close(fd);
                throw std::runtime_error("read /dev/urandom failed");
            }
            if (r == 0) {
                close(fd);
                throw std::runtime_error("EOF /dev/urandom");
            }
            off += (size_t)r;
        }
        close(fd);
    #endif
    }
};

int main() {
    try {
        // 5 test cases
        std::cout << "Hex token (16 bytes): " << Task128::secure_hex_token(16) << "\n";
        std::cout << "Random uint64 [1, 100]: " << Task128::secure_random_uint64(1, 100) << "\n";
        std::cout << "Hex token (32 bytes): " << Task128::secure_hex_token(32) << "\n";
        std::cout << "Hex token (0 bytes): " << Task128::secure_hex_token(0) << "\n";
        std::cout << "Random uint64 [0, 2^48]: " << Task128::secure_random_uint64(0, (1ULL << 48)) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}