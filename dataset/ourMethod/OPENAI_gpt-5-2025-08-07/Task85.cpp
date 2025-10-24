#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <cstring>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt")
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #include <stdlib.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/random.h>
  #include <sys/syscall.h>
  #ifndef GRND_NONBLOCK
    #define GRND_NONBLOCK 0x0001
  #endif
#endif

namespace {
    bool csprng_fill(unsigned char* buf, size_t len) {
        if (buf == nullptr || len == 0) return true;
    #if defined(_WIN32)
        NTSTATUS st = BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(buf), static_cast<ULONG>(len), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        return st == 0;
    #elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
        arc4random_buf(buf, len);
        return true;
    #else
        size_t off = 0;
        while (off < len) {
            ssize_t r = getrandom(buf + off, len - off, 0);
            if (r < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            off += static_cast<size_t>(r);
        }
        return true;
    #endif
    }

    std::string random_ascii_letters(std::size_t length) {
        const std::size_t MAX_LEN = 1000000;
        if (length > MAX_LEN) {
            throw std::invalid_argument("length must be in range [0, 1000000]");
        }
        static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        static constexpr unsigned letters_len = 52;
        static constexpr unsigned limit = (256 / letters_len) * letters_len; // 208

        std::string out;
        out.reserve(length);

        std::vector<unsigned char> buf(64);
        std::size_t idx = 0, avail = 0;

        while (out.size() < length) {
            if (idx >= avail) {
                if (!csprng_fill(buf.data(), buf.size())) {
                    throw std::runtime_error("Secure RNG failure");
                }
                idx = 0;
                avail = buf.size();
            }
            unsigned v = buf[idx++];
            if (v < limit) {
                out.push_back(letters[v % letters_len]);
            }
        }
        return out;
    }
}

int main() {
    try {
        std::size_t tests[5] = {0, 1, 10, 32, 100};
        for (std::size_t t : tests) {
            std::string s = random_ascii_letters(t);
            std::cout << "len=" << t << " str=" << s << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}