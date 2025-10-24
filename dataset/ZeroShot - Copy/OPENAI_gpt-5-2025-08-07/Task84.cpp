#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <cstdint>
#include <cstring>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "Bcrypt.lib")
#elif defined(__linux__)
  #include <sys/random.h>
  #include <errno.h>
  #include <unistd.h>
  #include <fcntl.h>
#else
  #include <errno.h>
  #include <unistd.h>
  #include <fcntl.h>
#endif

static bool secure_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#elif defined(__linux__)
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(buf + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        off += (size_t)r;
    }
    if (off == len) return true;
    // Fallback to /dev/urandom if getrandom partially failed
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return false;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (r == 0) { close(fd); return false; }
        off += (size_t)r;
    }
    close(fd);
    return true;
#else
    size_t off = 0;
    int fd = open("/dev/urandom", O_RDONLY
    #ifdef O_CLOEXEC
        | O_CLOEXEC
    #endif
    );
    if (fd < 0) return false;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (r == 0) { close(fd); return false; }
        off += (size_t)r;
    }
    close(fd);
    return true;
#endif
}

static std::string to_hex(const uint8_t* data, size_t len) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(len * 2);
    for (size_t i = 0; i < len; ++i) {
        out[2*i]   = hex[(data[i] >> 4) & 0xF];
        out[2*i+1] = hex[data[i] & 0xF];
    }
    return out;
}

static size_t clamp_length(size_t n) {
    if (n < 16 || n > 64) return 32;
    return n;
}

std::string generateSessionId(size_t numBytes) {
    static std::unordered_set<std::string> issued;
    static std::mutex mtx;

    size_t n = clamp_length(numBytes);
    std::vector<uint8_t> buf(n);

    for (;;) {
        if (!secure_random_bytes(buf.data(), buf.size())) {
            // As a last resort, throw on failure to ensure no weak IDs
            throw std::runtime_error("CSPRNG failure");
        }
        std::string id = to_hex(buf.data(), buf.size());
        std::lock_guard<std::mutex> lock(mtx);
        if (issued.insert(id).second) {
            return id;
        }
        // Extremely unlikely collision; retry
    }
}

int main() {
    try {
        // 5 test cases
        std::cout << generateSessionId(32) << "\n";
        std::cout << generateSessionId(32) << "\n";
        std::cout << generateSessionId(32) << "\n";
        std::cout << generateSessionId(32) << "\n";
        std::cout << generateSessionId(32) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}