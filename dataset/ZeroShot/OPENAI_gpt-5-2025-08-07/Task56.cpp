#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <fcntl.h>
  #include <unistd.h>
#endif

static void secure_random_bytes(std::vector<uint8_t>& out) {
    if (out.empty()) return;
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(nullptr, out.data(), (ULONG)out.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status != 0) {
        throw std::runtime_error("BCryptGenRandom failed");
    }
#else
    // Read from /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("Failed to open /dev/urandom");
    }
    size_t total = 0;
    while (total < out.size()) {
        ssize_t r = read(fd, out.data() + total, out.size() - total);
        if (r < 0) {
            int err = errno;
            close(fd);
            throw std::runtime_error(std::string("read /dev/urandom failed: ") + std::strerror(err));
        }
        if (r == 0) {
            close(fd);
            throw std::runtime_error("read /dev/urandom returned 0");
        }
        total += static_cast<size_t>(r);
    }
    close(fd);
#endif
}

static std::string base64_url_encode(const uint8_t* data, size_t len) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // Base64 size: 4 * ceil(n/3)
    size_t out_len = 4 * ((len + 2) / 3);
    std::string out;
    out.reserve(out_len);

    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t b = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i+1]) << 8) |
                     (static_cast<uint32_t>(data[i+2]));
        out.push_back(tbl[(b >> 18) & 0x3F]);
        out.push_back(tbl[(b >> 12) & 0x3F]);
        out.push_back(tbl[(b >> 6) & 0x3F]);
        out.push_back(tbl[b & 0x3F]);
        i += 3;
    }

    if (i + 1 == len) {
        uint32_t b = (static_cast<uint32_t>(data[i]) << 16);
        out.push_back(tbl[(b >> 18) & 0x3F]);
        out.push_back(tbl[(b >> 12) & 0x3F]);
        out.push_back('=');
        out.push_back('=');
    } else if (i + 2 == len) {
        uint32_t b = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i+1]) << 8);
        out.push_back(tbl[(b >> 18) & 0x3F]);
        out.push_back(tbl[(b >> 12) & 0x3F]);
        out.push_back(tbl[(b >> 6) & 0x3F]);
        out.push_back('=');
    }

    // Make URL-safe: replace '+' -> '-', '/' -> '_', strip '='
    for (char& c : out) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    // Strip padding '='
    while (!out.empty() && out.back() == '=') out.pop_back();

    return out;
}

// Generates a URL-safe base64 token with cryptographically secure randomness.
// Ensures at least 32 bytes of entropy.
std::string generate_token(size_t num_bytes) {
    size_t n = num_bytes < 32 ? 32 : num_bytes;
    std::vector<uint8_t> buf(n);
    secure_random_bytes(buf);
    return base64_url_encode(buf.data(), buf.size());
}

int main() {
    size_t tests[5] = {32, 48, 64, 16, 0};
    for (int i = 0; i < 5; ++i) {
        std::string tok = generate_token(tests[i]);
        std::cout << "C++ Token " << (i+1) << " (" << (tests[i] < 32 ? 32 : tests[i]) << " bytes): " << tok << "\n";
    }
    return 0;
}