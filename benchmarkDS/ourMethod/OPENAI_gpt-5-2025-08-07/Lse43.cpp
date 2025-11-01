#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <limits>

static const char* BASE_DIR = "/safe";
static const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MiB

static bool isValidName(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (unsigned char c : name) {
        if (c == '/' || c == '\\' || c == '\0') return false;
        if (!(c == '.' || c == '_' || c == '-' ||
              (c >= '0' && c <= '9') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z'))) {
            return false;
        }
    }
    return true;
}

static std::string readFromSafe(const std::string& name, bool& ok) {
    ok = false;
    if (!isValidName(name)) {
        return {};
    }

    int dirfd = open(BASE_DIR, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) {
        return {};
    }

    int fd = openat(dirfd, name.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        close(dirfd);
        return {};
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        close(dirfd);
        return {};
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        close(dirfd);
        return {};
    }

    if (st.st_size > 0 && static_cast<unsigned long long>(st.st_size) > static_cast<unsigned long long>(MAX_FILE_SIZE)) {
        close(fd);
        close(dirfd);
        return {};
    }

    std::string out;
    if (st.st_size > 0 && st.st_size <= static_cast<off_t>(MAX_FILE_SIZE)) {
        out.reserve(static_cast<size_t>(st.st_size));
    }

    char buf[8192];
    size_t total = 0;
    while (true) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r < 0) {
            out.clear();
            close(fd);
            close(dirfd);
            return {};
        }
        if (r == 0) break;
        if (total + static_cast<size_t>(r) > MAX_FILE_SIZE) {
            out.clear();
            close(fd);
            close(dirfd);
            return {};
        }
        out.append(buf, buf + r);
        total += static_cast<size_t>(r);
    }

    close(fd);
    close(dirfd);
    ok = true;
    return out;
}

int main() {
    std::vector<std::string> tests = {
        "example.txt",
        "nonexistent.txt",
        "../passwd",
        "symlink",
        "nested.txt"
    };
    for (const auto& t : tests) {
        bool ok = false;
        std::string data = readFromSafe(t, ok);
        if (!ok) {
            std::cout << "Read " << t << ": error\n";
        } else {
            std::string preview = data.substr(0, 64);
            for (auto& ch : preview) {
                if (ch == '\n') ch = ' ';
            }
            std::cout << "Read " << t << ": " << data.size() << " bytes; preview: " << preview << "\n";
        }
    }
    return 0;
}