#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

// Ranges: digits, uppercase, lowercase, Latin-1 supplement, Cyrillic, Emoji
struct Range { uint32_t lo; uint32_t hi; };
static const std::vector<Range> RANGES = {
    {0x0030, 0x0039}, {0x0041, 0x005A}, {0x0061, 0x007A},
    {0x00C0, 0x00FF}, {0x0400, 0x04FF}, {0x1F600, 0x1F64F}
};

static std::string utf8_encode(uint32_t cp) {
    std::string out;
    if (cp <= 0x7F) {
        out.push_back(static_cast<char>(cp));
    } else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return out;
}

static bool is_surrogate(uint32_t cp) {
    return cp >= 0xD800 && cp <= 0xDFFF;
}

static std::string generate_random_unicode_string(size_t length) {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    std::uniform_int_distribution<size_t> rangeDist(0, RANGES.size() - 1);

    std::string result;
    size_t produced = 0;
    size_t attempts = 0;
    size_t maxAttempts = length * 50;
    while (produced < length && attempts < maxAttempts) {
        attempts++;
        const Range& r = RANGES[rangeDist(rng)];
        std::uniform_int_distribution<uint32_t> cpDist(r.lo, r.hi);
        uint32_t cp = cpDist(rng);
        if (is_surrogate(cp)) continue;
        result += utf8_encode(cp);
        produced++;
    }
    if (produced < length) {
        throw std::runtime_error("Failed to generate sufficient random Unicode characters");
    }
    return result;
}

static std::string to_codepoints_line(const std::string& utf8) {
    // Decode minimal UTF-8 to get code points for representation
    std::string out;
    const unsigned char* s = reinterpret_cast<const unsigned char*>(utf8.data());
    size_t i = 0, n = utf8.size();
    auto append_cp = [&](uint32_t cp) {
        char buf[16];
        if (cp <= 0xFFFF) {
            std::snprintf(buf, sizeof(buf), "U+%04X ", cp);
        } else {
            std::snprintf(buf, sizeof(buf), "U+%06X ", cp);
        }
        out += buf;
    };
    while (i < n) {
        uint32_t cp = 0;
        unsigned char c = s[i];
        if (c <= 0x7F) {
            cp = c; i += 1;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
            cp = ((c & 0x1F) << 6) | (s[i+1] & 0x3F);
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
            cp = ((c & 0x0F) << 12) | ((s[i+1] & 0x3F) << 6) | (s[i+2] & 0x3F);
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
            cp = ((c & 0x07) << 18) | ((s[i+1] & 0x3F) << 12) | ((s[i+2] & 0x3F) << 6) | (s[i+3] & 0x3F);
            i += 4;
        } else {
            // Invalid byte; skip
            i += 1;
            continue;
        }
        append_cp(cp);
    }
    if (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

static std::string create_temp_unicode_file(int length) {
    if (length < 1 || length > 1024) {
        throw std::invalid_argument("length must be in [1, 1024]");
    }

    std::string original = generate_random_unicode_string(static_cast<size_t>(length));
    std::string codepoints = to_codepoints_line(original);

    std::filesystem::path dir = std::filesystem::temp_directory_path();
    std::string templ = (dir / "task88_XXXXXX").string();

    std::vector<char> tmpl(templ.begin(), templ.end());
    tmpl.push_back('\0');

    int fd = mkstemp(tmpl.data()); // creates and opens with O_EXCL
    if (fd < 0) {
        throw std::runtime_error(std::string("mkstemp failed: ") + std::strerror(errno));
    }

    // Set restrictive permissions 0600
    (void)fchmod(fd, S_IRUSR | S_IWUSR);

    std::string content = "Original: " + original + "\n" + "Codepoints: " + codepoints + "\n";
    const char* data = content.data();
    size_t total = content.size();
    ssize_t written = 0;
    while (static_cast<size_t>(written) < total) {
        ssize_t w = ::write(fd, data + written, total - written);
        if (w < 0) {
            int err = errno;
            ::close(fd);
            throw std::runtime_error(std::string("write failed: ") + std::strerror(err));
        }
        written += w;
    }
    (void)fsync(fd);
    ::close(fd);

    return std::string(tmpl.data());
}

int main() {
    int tests[5] = {8, 16, 1, 32, 50};
    for (int t : tests) {
        try {
            std::string path = create_temp_unicode_file(t);
            std::cout << "Created file (length " << t << "): " << path << std::endl;
        } catch (const std::exception& ex) {
            std::cout << "Error for length " << t << ": " << ex.what() << std::endl;
        }
    }
    return 0;
}