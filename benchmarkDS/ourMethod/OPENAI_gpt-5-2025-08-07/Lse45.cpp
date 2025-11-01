#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const size_t MAX_SIZE = 1 << 20;       // 1 MiB
static const size_t MAX_COMPONENTS = 64;
static const size_t MAX_COMPONENT_LEN = 255;

static bool is_safe_component(const std::string& s) {
    if (s.empty() || s == "." || s == ".." || s.size() > MAX_COMPONENT_LEN) return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '_' || c == ' ')) {
            return false;
        }
    }
    return true;
}

static std::vector<std::string> split_components(const std::string& rel) {
    std::vector<std::string> comps;
    std::string cur;
    for (char c : rel) {
        if (c == '/') {
            if (!cur.empty()) {
                comps.push_back(cur);
                cur.clear();
            } else {
                // empty component
                comps.push_back(std::string());
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) comps.push_back(cur);
    return comps;
}

static int open_component_dir(int dirfd, const std::string& name) {
    return openat(dirfd, name.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
}

static int open_file_nofollow_all(int base_dirfd, const std::string& rel, std::string& err) {
    if (rel.empty() || rel.size() > 4096) {
        err = "Invalid path length";
        return -1;
    }
    if (!rel.empty() && rel[0] == '/') {
        err = "Absolute not allowed";
        return -1;
    }
    if (rel.find('\0') != std::string::npos) {
        err = "NUL found";
        return -1;
    }
    if (std::count(rel.begin(), rel.end(), '/') + 1 > (int)MAX_COMPONENTS) {
        err = "Too many components";
        return -1;
    }
    std::vector<std::string> comps = split_components(rel);
    if (comps.empty()) {
        err = "Empty path";
        return -1;
    }
    for (auto& c : comps) {
        if (!is_safe_component(c)) {
            err = "Invalid component";
            return -1;
        }
    }

    int dfd = base_dirfd;
    int fd = -1;
    // Traverse all but last as directories
    for (size_t i = 0; i + 1 < comps.size(); ++i) {
        int nd = open_component_dir(dfd, comps[i]);
        if (nd < 0) {
            err = "Failed to open directory";
            return -1;
        }
        dfd = nd;
    }

    // Open the final file component without following symlinks
    fd = openat(dfd, comps.back().c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    // Close intermediate directory fd if it is not the base dirfd
    if (dfd != base_dirfd) {
        close(dfd);
    }
    if (fd < 0) {
        err = "Failed to open file";
        return -1;
    }
    return fd;
}

static std::string read_safe_file(const std::string& base_dir, const std::string& requested_path) {
    // Open base directory without following symlinks
    int basefd = open(base_dir.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
    if (basefd < 0) {
        throw std::runtime_error("Base open failed");
    }

    std::string err;
    int fd = open_file_nofollow_all(basefd, requested_path, err);
    close(basefd);
    if (fd < 0) {
        throw std::runtime_error(err);
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        throw std::runtime_error("fstat failed");
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        throw std::runtime_error("Not a regular file");
    }
    if (st.st_size < 0 || static_cast<unsigned long long>(st.st_size) > MAX_SIZE) {
        close(fd);
        throw std::runtime_error("File too large");
    }

    size_t to_read = static_cast<size_t>(st.st_size);
    std::string out;
    out.resize(to_read);
    size_t off = 0;
    while (off < to_read) {
        ssize_t r = read(fd, &out[off], to_read - off);
        if (r < 0) {
            close(fd);
            throw std::runtime_error("read failed");
        }
        if (r == 0) break;
        off += static_cast<size_t>(r);
    }
    close(fd);
    out.resize(off);
    return out;
}

static void write_file(const std::string& path, const std::string& data) {
    // Ensure parent directories exist
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        // Create dirs recursively
        std::string cur;
        for (size_t i = 0; i < dir.size(); ++i) {
            char c = dir[i];
            cur.push_back(c);
            if (c == '/') {
                mkdir(cur.c_str(), 0700);
            }
        }
        mkdir(dir.c_str(), 0700);
    }
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd >= 0) {
        ssize_t w = write(fd, data.data(), data.size());
        (void)w;
        fsync(fd);
        close(fd);
    }
}

int main() {
    // Create a temporary safe directory
    char tmpl[] = "/tmp/safe_dir_XXXXXX";
    char* base = mkdtemp(tmpl);
    if (!base) {
        std::cout << "Initialization failed\n";
        return 0;
    }
    std::string baseDir(base);

    // Create test files
    write_file(baseDir + "/a.txt", "Hello from a.txt");
    mkdir((baseDir + "/sub").c_str(), 0700);
    write_file(baseDir + "/sub/x.txt", "Nested file content");

    // Outside file for symlink
    std::string outside = baseDir + "/../outside.txt";
    write_file(outside, "outside");
    // Create symlink if possible
    std::string evil_link = baseDir + "/evil_link";
    unlink(evil_link.c_str());
    symlink(outside.c_str(), evil_link.c_str());

    // Five tests
    std::vector<std::string> tests = {
        "a.txt",        // valid
        "sub/x.txt",    // valid nested
        "../etc/passwd",// traversal
        "evil_link",    // symlink
        "sub"           // directory
    };

    for (const auto& t : tests) {
        try {
            std::string data = read_safe_file(baseDir, t);
            std::string preview = data.substr(0, std::min<size_t>(80, data.size()));
            std::cout << "OK: " << t << " => " << preview << "\n";
        } catch (const std::exception&) {
            std::cout << "FAIL: " << t << "\n";
        }
    }

    return 0;
}