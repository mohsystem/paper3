#include <iostream>
#include <string>
#include <regex>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <cerrno>
#include <cstring>
#include <limits>
#include <dirent.h>

static const size_t MAX_SIZE_BYTES = 1024 * 1024; // 1 MiB

static bool isValidFileName(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    static const std::regex re("^[A-Za-z0-9._-]{1,255}$");
    return std::regex_match(name, re);
}

static bool ensureDir(const std::string& dir) {
    struct stat st{};
    if (stat(dir.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(dir.c_str(), 0755) == 0) {
        return true;
    }
    return false;
}

static bool safeWriteAtomic(const std::string& baseDir, const std::string& fileName, const std::string& content, std::string& err) {
    if (!isValidFileName(fileName)) {
        err = "Invalid file name.";
        return false;
    }
    if (!ensureDir(baseDir)) {
        err = "Failed to ensure base directory.";
        return false;
    }
    if (content.size() > MAX_SIZE_BYTES) {
        err = "Content too large.";
        return false;
    }

    std::string tempTemplate = baseDir + "/.tmp-XXXXXX";
    std::vector<char> tmpl(tempTemplate.begin(), tempTemplate.end());
    tmpl.push_back('\0');

    int tfd = mkstemp(tmpl.data());
    if (tfd < 0) {
        err = std::string("mkstemp failed: ") + std::strerror(errno);
        return false;
    }

    bool ok = true;
    ssize_t remaining = static_cast<ssize_t>(content.size());
    const char* data = content.data();
    while (remaining > 0) {
        ssize_t w = write(tfd, data, static_cast<size_t>(remaining));
        if (w < 0) {
            ok = false;
            err = std::string("write failed: ") + std::strerror(errno);
            break;
        }
        remaining -= w;
        data += w;
    }

    if (ok) {
        if (fsync(tfd) != 0) {
            ok = false;
            err = std::string("fsync failed: ") + std::strerror(errno);
        }
    }

    if (close(tfd) != 0) {
        ok = false;
        err = std::string("close failed: ") + std::strerror(errno);
    }

    std::string tempPath(tmpl.data());
    std::string target = baseDir + "/" + fileName;

    if (ok) {
        if (rename(tempPath.c_str(), target.c_str()) != 0) {
            ok = false;
            err = std::string("rename failed: ") + std::strerror(errno);
        }
    }

    if (!ok) {
        unlink(tempPath.c_str());
    }
    return ok;
}

static std::pair<bool, std::string> retrieveFileContent(const std::string& baseDir, const std::string& fileName) {
    if (!isValidFileName(fileName)) {
        return {false, "ERROR: Invalid file name format."};
    }

    struct stat st{};
    if (stat(baseDir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        return {false, "ERROR: Base directory does not exist."};
    }

    std::string path = baseDir + "/" + fileName;
    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = open(path.c_str(), flags);
    if (fd < 0) {
        return {false, std::string("ERROR: ") + std::strerror(errno)};
    }

    std::pair<bool, std::string> result;
    do {
        struct stat fst{};
        if (fstat(fd, &fst) != 0) {
            result = {false, std::string("ERROR: ") + std::strerror(errno)};
            break;
        }
        if (!S_ISREG(fst.st_mode)) {
            result = {false, "ERROR: Not a regular file."};
            break;
        }
        if (fst.st_size < 0 || static_cast<size_t>(fst.st_size) > MAX_SIZE_BYTES) {
            result = {false, "ERROR: File too large."};
            break;
        }

        size_t toRead = static_cast<size_t>(fst.st_size);
        std::vector<char> buf;
        buf.reserve(toRead);
        const size_t chunk = 65536;
        size_t total = 0;
        while (total < toRead) {
            size_t want = std::min(chunk, toRead - total);
            std::vector<char> tmp(want);
            ssize_t r = read(fd, tmp.data(), want);
            if (r < 0) {
                result = {false, std::string("ERROR: ") + std::strerror(errno)};
                close(fd);
                return result;
            }
            if (r == 0) break;
            buf.insert(buf.end(), tmp.begin(), tmp.begin() + r);
            total += static_cast<size_t>(r);
        }
        std::string text(buf.begin(), buf.end());
        result = {true, "OK: " + text};
    } while (false);

    close(fd);
    return result;
}

int main() {
    std::string baseDir = "server_files";
    std::string err;
    if (!ensureDir(baseDir)) {
        std::cout << "Setup error: failed to ensure base directory\n";
        return 1;
    }
    if (!safeWriteAtomic(baseDir, "sample1.txt", "Hello from sample1.\nThis is a test file.", err)) {
        std::cout << "Setup error: " << err << "\n";
        return 1;
    }
    if (!safeWriteAtomic(baseDir, "safe-2.log", "Log line 1\nLog line 2\n", err)) {
        std::cout << "Setup error: " << err << "\n";
        return 1;
    }

    // 5 test cases
    const char* tests[] = {
        "sample1.txt",    // valid
        "../secret.txt",  // invalid traversal attempt
        "/etc/passwd",    // invalid absolute path
        "missing.txt",    // non-existent
        "safe-2.log"      // valid
    };

    for (const char* tf : tests) {
        auto res = retrieveFileContent(baseDir, tf);
        std::cout << "Request: '" << tf << "' => " << res.second << "\n";
    }
    return 0;
}