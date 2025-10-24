#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__)
#include <stdlib.h>
#elif defined(__linux__)
#include <sys/random.h>
#include <errno.h>
#include <unistd.h>
#else
#include <random>
#endif

namespace {
    constexpr size_t MIN_BYTES = 16;
    constexpr size_t MAX_BYTES = 64;

    bool getRandomBytes(uint8_t* buf, size_t len) {
        if (buf == nullptr || len == 0) return false;
    #ifdef _WIN32
        NTSTATUS status = BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(buf), static_cast<ULONG>(len), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        return status == 0;
    #elif defined(__APPLE__)
        arc4random_buf(buf, len);
        return true;
    #elif defined(__linux__)
        size_t total = 0;
        while (total < len) {
            ssize_t n = getrandom(buf + total, len - total, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                return false;
            }
            total += static_cast<size_t>(n);
        }
        return true;
    #else
        // Fallback: std::random_device (implementation-defined; may use OS CSPRNG)
        std::random_device rd;
        size_t i = 0;
        while (i + sizeof(unsigned int) <= len) {
            unsigned int v = rd();
            std::memcpy(buf + i, &v, sizeof(v));
            i += sizeof(v);
        }
        if (i < len) {
            unsigned int v = rd();
            std::memcpy(buf + i, &v, len - i);
        }
        return true;
    #endif
    }

    std::string base64urlNoPad(const std::vector<uint8_t>& data) {
        static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        const size_t n = data.size();
        std::string out;
        out.reserve(((n + 2) / 3) * 4); // upper bound

        size_t i = 0;
        while (i + 3 <= n) {
            uint32_t v = (static_cast<uint32_t>(data[i]) << 16) |
                         (static_cast<uint32_t>(data[i + 1]) << 8) |
                         (static_cast<uint32_t>(data[i + 2]));
            out.push_back(tbl[(v >> 18) & 0x3F]);
            out.push_back(tbl[(v >> 12) & 0x3F]);
            out.push_back(tbl[(v >> 6) & 0x3F]);
            out.push_back(tbl[v & 0x3F]);
            i += 3;
        }
        size_t rem = n - i;
        if (rem == 1) {
            uint32_t v = (static_cast<uint32_t>(data[i]) << 16);
            out.push_back(tbl[(v >> 18) & 0x3F]);
            out.push_back(tbl[(v >> 12) & 0x3F]);
            // no padding
        } else if (rem == 2) {
            uint32_t v = (static_cast<uint32_t>(data[i]) << 16) |
                         (static_cast<uint32_t>(data[i + 1]) << 8);
            out.push_back(tbl[(v >> 18) & 0x3F]);
            out.push_back(tbl[(v >> 12) & 0x3F]);
            out.push_back(tbl[(v >> 6) & 0x3F]);
            // no padding
        }
        return out;
    }

    std::string generateSessionId(size_t numBytes) {
        if (numBytes < MIN_BYTES || numBytes > MAX_BYTES) {
            throw std::invalid_argument("numBytes must be between 16 and 64");
        }
        std::vector<uint8_t> bytes(numBytes);
        if (!getRandomBytes(bytes.data(), bytes.size())) {
            throw std::runtime_error("CSPRNG failure");
        }
        return base64urlNoPad(bytes);
    }
}

int main() {
    try {
        std::cout << generateSessionId(16) << "\n";
        std::cout << generateSessionId(24) << "\n";
        std::cout << generateSessionId(32) << "\n";
        std::cout << generateSessionId(48) << "\n";
        std::cout << generateSessionId(64) << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}