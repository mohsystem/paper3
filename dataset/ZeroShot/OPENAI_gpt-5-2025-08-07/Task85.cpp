#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
static bool secure_random_bytes(uint8_t* buf, size_t len) {
    return BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
}
#elif defined(__APPLE__)
#include <stdlib.h>
static bool secure_random_bytes(uint8_t* buf, size_t len) {
    arc4random_buf(buf, len);
    return true;
}
#elif defined(__linux__)
#include <sys/random.h>
#include <unistd.h>
#include <fcntl.h>
static bool secure_random_bytes(uint8_t* buf, size_t len) {
    ssize_t r = getrandom(buf, len, 0);
    if (r == (ssize_t)len) return true;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n <= 0) { close(fd); return false; }
        total += (size_t)n;
    }
    close(fd);
    return true;
}
#else
#include <unistd.h>
#include <fcntl.h>
static bool secure_random_bytes(uint8_t* buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n <= 0) { close(fd); return false; }
        total += (size_t)n;
    }
    close(fd);
    return true;
}
#endif

static std::string generate_random_string(size_t length) {
    const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t alphabet = sizeof(letters) - 1; // 52
    if (length == 0) return std::string();
    std::string out;
    out.reserve(length);
    const uint32_t m = 256 - (256 % alphabet); // 208 to avoid modulo bias

    std::vector<uint8_t> buf(128);
    size_t idx = buf.size();
    while (out.size() < length) {
        if (idx >= buf.size()) {
            if (!secure_random_bytes(buf.data(), buf.size())) {
                throw std::runtime_error("secure randomness unavailable");
            }
            idx = 0;
        }
        uint8_t b = buf[idx++];
        if (b >= m) continue;
        out.push_back(letters[b % alphabet]);
    }
    return out;
}

int main() {
    size_t tests[5] = {0, 1, 10, 32, 64};
    for (size_t len : tests) {
        std::string s = generate_random_string(len);
        std::cout << len << ": " << s << std::endl;
    }
    return 0;
}