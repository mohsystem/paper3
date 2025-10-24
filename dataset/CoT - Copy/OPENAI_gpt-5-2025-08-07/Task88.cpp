#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <share.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
#endif

/*
Chain-of-Through in code (comments only):
1) Problem: Create UTF-8 temp file, generate random string, convert to Unicode code point notation, write, return path.
2) Security: Use random_device/MT for randomness, secure unique file creation (O_CREAT|O_EXCL or CREATE_NEW), 0600 on POSIX.
3) Implementation: Cross-platform temp dir and exclusive file creation, avoid race conditions.
4) Review: Check all errors, close handles, avoid UB with Unicode handling.
5) Secure output: Final function returns path; main demonstrates 5 test cases.
*/

// Predefined code points for randomness
static const uint32_t CODE_POINTS[] = {
    // ASCII upper, lower, digits
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9',
    // Extended BMP
    0x00E9, 0x03A9, 0x0416, 0x4F60, 0x597D, 0x6F22, 0x20AC, 0x2603,
    // Supplementary
    0x1F642, 0x1F680, 0x1F9E9, 0x1D6D1
};

static size_t CODE_POINTS_COUNT = sizeof(CODE_POINTS)/sizeof(CODE_POINTS[0]);

static int sanitize_length(int length) {
    if (length < 0) return 16;
    if (length > 4096) return 4096;
    return length;
}

static std::vector<uint32_t> random_codepoints(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, CODE_POINTS_COUNT - 1);
    std::vector<uint32_t> cps;
    cps.reserve((size_t)length);
    for (int i = 0; i < length; ++i) {
        cps.push_back(CODE_POINTS[dist(gen)]);
    }
    return cps;
}

static std::string to_unicode_points(const std::vector<uint32_t>& cps) {
    std::ostringstream oss;
    bool first = true;
    for (uint32_t cp : cps) {
        if (!first) oss << ' ';
        first = false;
        if (cp <= 0xFFFFu) {
            oss << "U+" << std::uppercase << std::setw(4) << std::setfill('0') << std::hex << cp;
        } else {
            oss << "U+" << std::uppercase << std::setw(6) << std::setfill('0') << std::hex << cp;
        }
    }
    return oss.str();
}

#ifdef _WIN32
static std::string get_temp_dir() {
    char buf[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, buf);
    if (n == 0 || n > MAX_PATH) return ".\\";
    return std::string(buf);
}

static bool write_all_h(HANDLE h, const char* data, size_t len) {
    const char* p = data;
    size_t remaining = len;
    while (remaining > 0) {
        DWORD wrote = 0;
        if (!WriteFile(h, p, (DWORD)std::min<size_t>(remaining, 1 << 20), &wrote, nullptr)) {
            return false;
        }
        if (wrote == 0) return false;
        p += wrote;
        remaining -= wrote;
    }
    return true;
}
#else
static std::string get_temp_dir() {
    const char* t = getenv("TMPDIR");
    if (!t || !*t) t = getenv("TEMP");
    if (!t || !*t) t = getenv("TMP");
    if (!t || !*t) t = "/tmp";
    return std::string(t);
}

static bool write_all_fd(int fd, const char* data, size_t len) {
    const char* p = data;
    size_t remaining = len;
    while (remaining > 0) {
        ssize_t w = write(fd, p, remaining);
        if (w <= 0) return false;
        p += (size_t)w;
        remaining -= (size_t)w;
    }
    return true;
}
#endif

// Public API
static std::string createTempUnicodeFile(int length) {
    int safeLen = sanitize_length(length);
    std::vector<uint32_t> cps = random_codepoints(safeLen);
    std::string unicode_repr = to_unicode_points(cps);

#ifdef _WIN32
    std::string dir = get_temp_dir();
    // Generate a unique name and create with exclusive flag
    std::random_device rd;
    for (int attempt = 0; attempt < 25; ++attempt) {
        unsigned int r1 = rd();
        unsigned int r2 = rd();
        std::ostringstream name;
        name << "task88_" << std::hex << std::uppercase << r1 << "_" << r2 << ".txt";
        std::string path = dir + name.str();

        HANDLE h = CreateFileA(
            path.c_str(),
            GENERIC_WRITE,
            0,                      // no sharing
            nullptr,
            CREATE_NEW,             // exclusive create
            FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_ARCHIVE,
            nullptr
        );
        if (h == INVALID_HANDLE_VALUE) {
            DWORD err = GetLastError();
            if (err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS) {
                continue;
            } else {
                continue;
            }
        }
        // Write ASCII/UTF-8 content
        bool ok = write_all_h(h, unicode_repr.c_str(), unicode_repr.size());
        CloseHandle(h);
        if (!ok) {
            // Cleanup on failure
            DeleteFileA(path.c_str());
            continue;
        }
        return path;
    }
    return std::string();
#else
    std::string dir = get_temp_dir();
    std::string tmpl = dir;
    if (!tmpl.empty() && tmpl.back() != '/') tmpl.push_back('/');
    tmpl += "task88_XXXXXX";
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    int fd = mkstemp(buf.data());
    if (fd == -1) {
        return std::string();
    }
    // Restrict permissions to 0600
    fchmod(fd, S_IRUSR | S_IWUSR);
    bool ok = write_all_fd(fd, unicode_repr.c_str(), unicode_repr.size());
    close(fd);
    if (!ok) {
        unlink(buf.data());
        return std::string();
    }
    return std::string(buf.data());
#endif
}

int main() {
    int tests[5] = {0, 1, 5, 10, 50};
    for (int t : tests) {
        std::string path = createTempUnicodeFile(t);
        if (!path.empty()) {
            std::cout << "Created temp file (len=" << t << "): " << path << "\n";
        } else {
            std::cout << "Failed to create temp file (len=" << t << ")\n";
        }
    }
    return 0;
}