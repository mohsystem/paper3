#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#if defined(_WIN32)
  #define NOMINMAX
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #if defined(__linux__)
    #include <sys/random.h>
  #endif
#endif

static const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

static bool secure_random_bytes(uint8_t* buf, size_t len) {
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0;
#else
  #if defined(__linux__)
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
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return false;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        total += static_cast<size_t>(n);
    }
    close(fd);
    return true;
  #endif
#endif
}

static size_t random_index(size_t bound) {
    if (bound == 0) throw std::invalid_argument("bound must be > 0");
    uint64_t x;
    const uint64_t limit = (std::numeric_limits<uint64_t>::max() / bound) * bound;
    for (;;) {
        if (!secure_random_bytes(reinterpret_cast<uint8_t*>(&x), sizeof(x))) {
            throw std::runtime_error("Failed to obtain secure random bytes");
        }
        if (x < limit) {
            return static_cast<size_t>(x % bound);
        }
    }
}

static std::string generate_random_string(size_t length, const std::string& alphabet) {
    std::string out;
    out.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        size_t idx = random_index(alphabet.size());
        out.push_back(alphabet[idx]);
    }
    return out;
}

static std::string to_unicode_escapes(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 6);
    char buf[7] = {0};
    for (unsigned char c : s) {
        std::snprintf(buf, sizeof(buf), "\\u%04X", static_cast<unsigned int>(c));
        out.append(buf);
    }
    return out;
}

static std::string create_temp_unicode_file(size_t length) {
    if (length == 0) throw std::invalid_argument("length must be > 0");

    std::string random = generate_random_string(length, ALPHABET);
    std::string escaped = to_unicode_escapes(random);

#if defined(_WIN32)
    char tempPath[MAX_PATH];
    DWORD tplen = GetTempPathA(MAX_PATH, tempPath);
    if (tplen == 0 || tplen > MAX_PATH) {
        throw std::runtime_error("GetTempPathA failed");
    }
    char tempFile[MAX_PATH];
    if (GetTempFileNameA(tempPath, "t88", 0, tempFile) == 0) {
        throw std::runtime_error("GetTempFileNameA failed");
    }
    // Write file in binary mode with UTF-8 bytes (ASCII subset)
    std::ofstream ofs(tempFile, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        throw std::runtime_error("Failed to open temp file for writing");
    }
    ofs.write(escaped.data(), static_cast<std::streamsize>(escaped.size()));
    ofs.flush();
    if (!ofs) {
        throw std::runtime_error("Failed to write temp file");
    }
    ofs.close();
    return std::string(tempFile);
#else
    const char* dir = std::getenv("TMPDIR");
    if (!dir || *dir == '\0') dir = "/tmp";
    std::string tmpl = std::string(dir) + "/task88_XXXXXX";
    std::vector<char> pathbuf(tmpl.begin(), tmpl.end());
    pathbuf.push_back('\0');

    int fd = mkstemp(pathbuf.data());
    if (fd == -1) {
        throw std::runtime_error("mkstemp failed");
    }

    // Restrict permissions to 0600
    fchmod(fd, S_IRUSR | S_IWUSR);

    // Write content
    size_t total = 0;
    const char* data = escaped.c_str();
    size_t len = escaped.size();
    while (total < len) {
        ssize_t w = write(fd, data + total, len - total);
        if (w < 0) {
            if (errno == EINTR) continue;
            close(fd);
            throw std::runtime_error("Failed to write to temp file");
        }
        total += static_cast<size_t>(w);
    }
    if (fsync(fd) != 0) {
        close(fd);
        throw std::runtime_error("fsync failed");
    }
    close(fd);

    return std::string(pathbuf.data());
#endif
}

int main() {
    try {
        std::cout << create_temp_unicode_file(8) << std::endl;
        std::cout << create_temp_unicode_file(12) << std::endl;
        std::cout << create_temp_unicode_file(1) << std::endl;
        std::cout << create_temp_unicode_file(32) << std::endl;
        std::cout << create_temp_unicode_file(64) << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}