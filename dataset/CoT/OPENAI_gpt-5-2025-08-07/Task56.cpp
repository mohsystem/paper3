// Chain-of-Through process in code generation:
// 1) Problem understanding: Generate a cryptographically secure, URL-safe token string for authentication.
// 2) Security requirements: Use OS CSPRNG, sufficient entropy (>=128 bits), URL-safe Base64 without padding.
// 3) Secure coding generation: Validate input size, robust I/O with /dev/urandom (POSIX) or BCryptGenRandom (Windows), avoid undefined behavior.
// 4) Code review: Checked RNG source, error handling, and encoding correctness.
// 5) Secure code output: Code uses strong RNG and safe encoding with input validation.

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>

#ifdef _WIN32
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "Bcrypt.lib")
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <errno.h>
#endif

static bool get_secure_random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buf, static_cast<ULONG>(len), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    // Use /dev/urandom on POSIX
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (r == 0) {
            close(fd);
            return false;
        }
        off += static_cast<size_t>(r);
    }
    close(fd);
    return true;
#endif
}

static std::string base64url_encode(const uint8_t* data, size_t len) {
    static const char* alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    out.reserve((len * 4 + 2) / 3);

    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t triple = (static_cast<uint32_t>(data[i]) << 16) |
                          (static_cast<uint32_t>(data[i + 1]) << 8) |
                           static_cast<uint32_t>(data[i + 2]);
        out.push_back(alphabet[(triple >> 18) & 0x3F]);
        out.push_back(alphabet[(triple >> 12) & 0x3F]);
        out.push_back(alphabet[(triple >> 6) & 0x3F]);
        out.push_back(alphabet[triple & 0x3F]);
        i += 3;
    }

    size_t rem = len - i;
    if (rem == 1) {
        uint8_t b0 = data[i];
        out.push_back(alphabet[(b0 >> 2) & 0x3F]);
        out.push_back(alphabet[(b0 & 0x03) << 4]);
        // No padding for URL-safe variant
    } else if (rem == 2) {
        uint8_t b0 = data[i];
        uint8_t b1 = data[i + 1];
        out.push_back(alphabet[(b0 >> 2) & 0x3F]);
        out.push_back(alphabet[((b0 & 0x03) << 4) | (b1 >> 4)]);
        out.push_back(alphabet[(b1 & 0x0F) << 2]);
        // No padding for URL-safe variant
    }

    return out;
}

std::string generateAuthToken(size_t numBytes) {
    if (numBytes < 16 || numBytes > 64) {
        throw std::invalid_argument("numBytes must be between 16 and 64");
    }
    std::vector<uint8_t> buf(numBytes);
    if (!get_secure_random_bytes(buf.data(), buf.size())) {
        throw std::runtime_error("Failed to obtain secure random bytes");
    }
    return base64url_encode(buf.data(), buf.size());
}

int main() {
    try {
        std::vector<size_t> tests = {16, 24, 32, 48, 64};
        for (auto n : tests) {
            auto t = generateAuthToken(n);
            std::cout << "Bytes=" << n << " Token=" << t << " Length=" << t.size() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}