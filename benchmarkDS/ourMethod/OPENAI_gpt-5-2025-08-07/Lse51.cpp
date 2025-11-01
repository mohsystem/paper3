#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

// Security helpers
static bool is_regular_file_fd(int fd) {
    struct stat st{};
    if (fstat(fd, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

static bool ensure_dirfd(const std::string &base, int &dirfd_out, std::string &err) {
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int dfd = open(base.c_str(), flags);
    if (dfd < 0) {
        err = "open base failed";
        return false;
    }
    dirfd_out = dfd;
    return true;
}

static bool path_is_safe_relative(const fs::path &rel) {
    if (rel.is_absolute()) return false;
    for (const auto &p : rel) {
        if (p == "..") return false;
    }
    return true;
}

static bool mkdirat_parents(int dirfd, const fs::path &rel, std::string &err) {
    fs::path accum;
    for (const auto &p : rel) {
        accum /= p;
        std::string name = p.string();
        if (name.empty()) continue;
        if (mkdirat(dirfd, accum.c_str(), 0700) != 0) {
            if (errno == EEXIST) {
                // Verify not symlink and is directory where possible
                int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
                flags |= O_NOFOLLOW;
#endif
                int d = openat(dirfd, accum.c_str(), flags);
                if (d < 0) {
                    if (errno == ELOOP) {
                        err = "symlink encountered in path";
                        return false;
                    }
                    // if it's a directory we can't open without O_DIRECTORY, try with O_DIRECTORY
                    int d2 = openat(dirfd, accum.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC
#ifdef O_NOFOLLOW
                                    | O_NOFOLLOW
#endif
                    );
                    if (d2 < 0) {
                        err = "existing path not directory";
                        return false;
                    }
                    close(d2);
                } else {
                    // open succeeded, check it's a dir
                    struct stat st{};
                    if (fstat(d, &st) != 0 || !S_ISDIR(st.st_mode)) {
                        close(d);
                        err = "existing path not directory";
                        return false;
                    }
                    close(d);
                }
                continue;
            } else {
                err = "mkdirat failed";
                return false;
            }
        }
    }
    return true;
}

static std::string rand_hex(size_t nbytes) {
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 255);
    std::string out;
    out.resize(nbytes * 2);
    static const char *hex = "0123456789abcdef";
    for (size_t i = 0; i < nbytes; ++i) {
        unsigned char b = static_cast<unsigned char>(dist(rd));
        out[2 * i] = hex[b >> 4];
        out[2 * i + 1] = hex[b & 0x0F];
    }
    return out;
}

static bool secure_write_file(const std::string &base, const std::string &relpath, const std::string &content, std::string &err) {
    fs::path rel(relpath);
    if (!path_is_safe_relative(rel)) { err = "unsafe path"; return false; }

    int dirfd = -1;
    if (!ensure_dirfd(base, dirfd, err)) return false;

    // Create parent directories
    fs::path parent = rel.parent_path();
    if (!parent.empty()) {
        if (!mkdirat_parents(dirfd, parent, err)) {
            close(dirfd);
            return false;
        }
    }

    // Create temp file
    std::string tmpName = rel.filename().string() + ".tmp." + rand_hex(8);
    fs::path tmpPath = parent / tmpName;

    int oflags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC;
#ifdef O_NOFOLLOW
    oflags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, tmpPath.c_str(), oflags, 0600);
    if (fd < 0) {
        close(dirfd);
        err = "openat tmp failed";
        return false;
    }

    // Write data
    const char *buf = content.data();
    size_t toWrite = content.size();
    while (toWrite > 0) {
        ssize_t w = write(fd, buf, toWrite > 65536 ? 65536 : toWrite);
        if (w < 0) {
            close(fd);
            close(dirfd);
            err = "write failed";
            return false;
        }
        buf += w;
        toWrite -= static_cast<size_t>(w);
    }
    if (fsync(fd) != 0) {
        close(fd);
        close(dirfd);
        err = "fsync failed";
        return false;
    }
    if (close(fd) != 0) {
        close(dirfd);
        err = "close failed";
        return false;
    }

    // Atomic rename
    if (renameat(dirfd, tmpPath.c_str(), dirfd, rel.c_str()) != 0) {
        // cleanup temp
        unlinkat(dirfd, tmpPath.c_str(), 0);
        close(dirfd);
        err = "rename failed";
        return false;
    }

    close(dirfd);
    return true;
}

static bool secure_read_file(const std::string &base, const std::string &relpath, std::string &out, std::string &err) {
    fs::path rel(relpath);
    if (!path_is_safe_relative(rel)) { err = "unsafe path"; return false; }

    int dirfd = -1;
    if (!ensure_dirfd(base, dirfd, err)) return false;

    int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, rel.c_str(), flags);
    if (fd < 0) {
        close(dirfd);
        err = "openat read failed";
        return false;
    }
    if (!is_regular_file_fd(fd)) {
        close(fd);
        close(dirfd);
        err = "not a regular file";
        return false;
    }
    out.clear();
    char buf[65536];
    ssize_t r;
    while ((r = read(fd, buf, sizeof buf)) > 0) {
        out.append(buf, buf + r);
        if (out.size() > 10 * 1024 * 1024) { // 10MB cap
            close(fd);
            close(dirfd);
            err = "file too large";
            return false;
        }
    }
    if (r < 0) {
        close(fd);
        close(dirfd);
        err = "read failed";
        return false;
    }
    close(fd);
    close(dirfd);
    return true;
}

int main() {
    const std::string base = "/tmp/unpack_cpp";
    fs::create_directories(base);

    // Test 1: write a.txt
    {
        std::string err;
        bool ok = secure_write_file(base, "a.txt", "hello cpp", err);
        std::cout << "Test1 write a.txt: " << (ok ? "OK" : ("FAIL " + err)) << "\n";
    }

    // Test 2: write nested sub/b.txt
    {
        std::string err;
        bool ok = secure_write_file(base, "sub/b.txt", "nested content", err);
        std::cout << "Test2 write sub/b.txt: " << (ok ? "OK" : ("FAIL " + err)) << "\n";
    }

    // Test 3: traversal attempt ../evil.txt
    {
        std::string err;
        bool ok = secure_write_file(base, "../evil.txt", "nope", err);
        std::cout << "Test3 traversal: " << (ok ? "UNEXPECTED OK" : "Blocked") << "\n";
    }

    // Test 4: absolute path attempt
    {
        std::string err;
        bool ok = secure_write_file(base, "/etc/passwd", "nope", err);
        std::cout << "Test4 absolute: " << (ok ? "UNEXPECTED OK" : "Blocked") << "\n";
    }

    // Test 5: read back sub/b.txt
    {
        std::string data, err;
        bool ok = secure_read_file(base, "sub/b.txt", data, err);
        std::cout << "Test5 read sub/b.txt: " << (ok ? ("OK len=" + std::to_string(data.size())) : ("FAIL " + err)) << "\n";
    }

    return 0;
}