#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <cerrno>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

static bool isValidUserId(const std::string& userId) {
    if (userId.empty() || userId.size() > 128) return false;
    for (unsigned char c : userId) {
        if (c < 33 || c > 126) return false; // printable non-space ASCII
    }
    return true;
}

static bool getSecureRandomBytes(uint8_t* buf, size_t len) {
    if (buf == nullptr || len == 0) return false;
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(buf),
                                      static_cast<ULONG>(len),
                                      BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    // best-effort set close-on-exec
    (void)fcntl(fd, F_SETFD, FD_CLOEXEC);

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
    if (len == 0) return std::string();

    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i + 3 <= len) {
        uint32_t v = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i + 1]) << 8) |
                     (static_cast<uint32_t>(data[i + 2]));
        out.push_back(alphabet[(v >> 18) & 0x3F]);
        out.push_back(alphabet[(v >> 12) & 0x3F]);
        out.push_back(alphabet[(v >> 6) & 0x3F]);
        out.push_back(alphabet[v & 0x3F]);
        i += 3;
    }

    size_t rem = len - i;
    if (rem == 1) {
        uint32_t v = (static_cast<uint32_t>(data[i]) << 16);
        out.push_back(alphabet[(v >> 18) & 0x3F]);
        out.push_back(alphabet[(v >> 12) & 0x3F]);
        // No padding for base64url
    } else if (rem == 2) {
        uint32_t v = (static_cast<uint32_t>(data[i]) << 16) |
                     (static_cast<uint32_t>(data[i + 1]) << 8);
        out.push_back(alphabet[(v >> 18) & 0x3F]);
        out.push_back(alphabet[(v >> 12) & 0x3F]);
        out.push_back(alphabet[(v >> 6) & 0x3F]);
        // No padding for base64url
    }

    return out;
}

std::string generateAuthToken(const std::string& userId, size_t entropyBytes) {
    if (!isValidUserId(userId)) {
        return std::string(); // invalid input
    }
    if (entropyBytes < 16 || entropyBytes > 64) {
        return std::string(); // enforce reasonable bounds
    }

    std::vector<uint8_t> rnd(entropyBytes);
    if (!getSecureRandomBytes(rnd.data(), rnd.size())) {
        return std::string(); // RNG failure
    }

    std::string token = base64url_encode(rnd.data(), rnd.size());

    // Clear sensitive random buffer
    volatile uint8_t* p = reinterpret_cast<volatile uint8_t*>(rnd.data());
    for (size_t i = 0; i < rnd.size(); ++i) {
        p[i] = 0;
    }

    return token;
}

int main() {
    struct TestCase { std::string uid; size_t bytes; };
    std::vector<TestCase> tests = {
        {"alice", 32},
        {"bob_01", 16},
        {"USER-3", 24},
        {"john.doe@example.com", 48},
        {"Zed-Admin_2025", 64}
    };

    std::vector<std::string> tokens;
    tokens.reserve(tests.size());

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string t = generateAuthToken(tests[i].uid, tests[i].bytes);
        if (t.empty()) {
            std::cout << "Test " << (i + 1) << ": FAILED (generation error)\n";
        } else {
            tokens.push_back(t);
            std::cout << "Test " << (i + 1) << ": OK (token length=" << t.size() << ")\n";
        }
    }

    bool unique = true;
    for (size_t i = 0; i < tokens.size(); ++i) {
        for (size_t j = i + 1; j < tokens.size(); ++j) {
            if (tokens[i] == tokens[j]) {
                unique = false;
            }
        }
    }
    std::cout << "Uniqueness check: " << (unique ? "PASSED" : "FAILED") << "\n";
    return 0;
}