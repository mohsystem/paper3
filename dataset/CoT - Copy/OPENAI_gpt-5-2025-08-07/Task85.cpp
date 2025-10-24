#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#ifdef _MSC_VER
#pragma comment(lib, "bcrypt.lib")
#endif
#else
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __linux__
#include <sys/random.h>
#endif
#endif

static bool get_random_bytes(uint8_t* buf, size_t len) {
#ifdef _WIN32
    return BCryptGenRandom(NULL, reinterpret_cast<PUCHAR>(buf), static_cast<ULONG>(len), BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#else
#ifdef __linux__
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(buf + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        off += static_cast<size_t>(r);
    }
    if (off == len) return true;
#endif
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

std::string generate_random_string(size_t length) {
    static const char* letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const size_t alphabet = 52;
    std::string out;
    out.reserve(length);
    const uint8_t limit = static_cast<uint8_t>(256 - (256 % alphabet));
    while (out.size() < length) {
        uint8_t buffer[64];
        if (!get_random_bytes(buffer, sizeof(buffer))) {
            return std::string();
        }
        for (uint8_t b : buffer) {
            if (b < limit) {
                out.push_back(letters[b % alphabet]);
                if (out.size() == length) break;
            }
        }
    }
    return out;
}

int main() {
    std::vector<size_t> tests = {0, 1, 16, 32, 100};
    for (size_t t : tests) {
        std::string s = generate_random_string(t);
        std::cout << t << ": " << s << "\n";
    }
    return 0;
}