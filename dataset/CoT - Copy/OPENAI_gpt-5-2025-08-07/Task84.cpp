// Step 1-5: Secure session ID generator in C++
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <stdexcept>
#include <mutex>
#include <cstring>

#ifdef _WIN32
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "Bcrypt.lib")
#else
  #include <cstdio>
  #include <cerrno>
  #include <fcntl.h>
  #include <unistd.h>
#endif

namespace SecureSession {

static const size_t MIN_BYTES = 16;
static const size_t MAX_BYTES = 1024;
static const int MAX_ATTEMPTS = 1000;

static std::unordered_set<std::string> issued;
static std::mutex issued_mtx;

static void secure_random_bytes(unsigned char* buf, size_t len) {
#ifdef _WIN32
    if (BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) {
        throw std::runtime_error("BCryptGenRandom failed");
    }
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error("Failed to open /dev/urandom");
    }
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, buf + total, len - total);
        if (r < 0) {
            int e = errno;
            close(fd);
            throw std::runtime_error(std::string("read /dev/urandom failed: ") + std::strerror(e));
        }
        if (r == 0) {
            close(fd);
            throw std::runtime_error("read /dev/urandom EOF");
        }
        total += static_cast<size_t>(r);
    }
    close(fd);
#endif
}

static std::string base64url_encode(const unsigned char* data, size_t len) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= len) {
        unsigned int v = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(v >> 18) & 0x3F]);
        out.push_back(tbl[(v >> 12) & 0x3F]);
        out.push_back(tbl[(v >> 6) & 0x3F]);
        out.push_back(tbl[v & 0x3F]);
        i += 3;
    }
    size_t rem = len - i;
    if (rem == 1) {
        unsigned int v = (data[i] << 16);
        out.push_back(tbl[(v >> 18) & 0x3F]);
        out.push_back(tbl[(v >> 12) & 0x3F]);
        // no padding; omit last 2
    } else if (rem == 2) {
        unsigned int v = (data[i] << 16) | (data[i+1] << 8);
        out.push_back(tbl[(v >> 18) & 0x3F]);
        out.push_back(tbl[(v >> 12) & 0x3F]);
        out.push_back(tbl[(v >> 6) & 0x3F]);
        // no padding; omit last 1
    }
    return out;
}

std::string generateSessionId(size_t numBytes) {
    if (numBytes < MIN_BYTES || numBytes > MAX_BYTES) {
        throw std::invalid_argument("numBytes must be between 16 and 1024");
    }
    std::vector<unsigned char> buf(numBytes);
    for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
        secure_random_bytes(buf.data(), buf.size());
        std::string id = base64url_encode(buf.data(), buf.size());
        {
            std::lock_guard<std::mutex> lk(issued_mtx);
            if (issued.insert(id).second) {
                return id;
            }
        }
    }
    throw std::runtime_error("Unable to generate a unique session ID after max attempts");
}

} // namespace SecureSession

int main() {
    // 5 test cases
    try {
        std::cout << SecureSession::generateSessionId(16) << "\n";
        std::cout << SecureSession::generateSessionId(24) << "\n";
        std::cout << SecureSession::generateSessionId(32) << "\n";
        std::cout << SecureSession::generateSessionId(48) << "\n";
        std::cout << SecureSession::generateSessionId(64) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}