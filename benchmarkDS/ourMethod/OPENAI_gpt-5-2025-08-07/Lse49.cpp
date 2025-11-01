#include <cerrno>
#include <cinttypes>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

static const size_t TAR_BLOCK = 512;
static const off_t MAX_FILE_SIZE = 1000000000LL; // 1GB

static ssize_t read_full(int fd, void* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t r = ::read(fd, (char*)buf + off, n - off);
        if (r == 0) break;
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)r;
    }
    return (ssize_t)off;
}

static bool is_zero_block(const unsigned char* b) {
    for (size_t i = 0; i < TAR_BLOCK; ++i) {
        if (b[i] != 0) return false;
    }
    return true;
}

static std::string read_tar_string(const unsigned char* hdr, size_t off, size_t len) {
    size_t end = off + len;
    size_t i = off;
    for (; i < end; ++i) {
        if (hdr[i] == 0) break;
    }
    return std::string((const char*)hdr + off, (size_t)(i - off));
}

static bool parse_octal(const unsigned char* hdr, size_t off, size_t len, off_t& out) {
    out = 0;
    size_t i = off;
    size_t end = off + len;
    // skip leading spaces and nulls
    while (i < end && (hdr[i] == 0 || hdr[i] == ' ')) i++;
    for (; i < end; ++i) {
        unsigned char c = hdr[i];
        if (c == 0 || c == ' ') break;
        if (c < '0' || c > '7') break;
        int digit = c - '0';
        if (out > (LLONG_MAX >> 3)) return false;
        out = (out << 3) + digit;
    }
    return true;
}

static bool is_safe_relpath(const std::string& p) {
    if (p.empty()) return false;
    if (p[0] == '/') return false;
    size_t i = 0;
    while (i < p.size()) {
        size_t j = p.find('/', i);
        std::string seg = (j == std::string::npos) ? p.substr(i) : p.substr(i, j - i);
        if (seg.empty() || seg == "." || seg == "..") return false;
        if (j == std::string::npos) break;
        i = j + 1;
    }
    return true;
}

static int open_dir_at(int dirfd, const char* name, int* out_fd) {
    int fd = openat(dirfd, name, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(fd);
        errno = ENOTDIR;
        return -1;
    }
    *out_fd = fd;
    return 0;
}

static int mkdirs_at(int basefd, const std::string& rel_dir, int* out_fd) {
    int current = dup(basefd);
    if (current < 0) return -1;
    if (rel_dir.empty()) {
        *out_fd = current;
        return 0;
    }
    size_t i = 0;
    while (i < rel_dir.size()) {
        size_t j = rel_dir.find('/', i);
        std::string seg = (j == std::string::npos) ? rel_dir.substr(i) : rel_dir.substr(i, j - i);
        if (seg.empty() || seg == "." || seg == "..") {
            close(current);
            errno = EINVAL;
            return -1;
        }
        int nextfd = openat(current, seg.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (nextfd < 0) {
            if (errno != ENOENT) {
                close(current);
                return -1;
            }
            if (mkdirat(current, seg.c_str(), 0700) != 0) {
                close(current);
                return -1;
            }
            nextfd = openat(current, seg.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
            if (nextfd < 0) {
                close(current);
                return -1;
            }
        }
        close(current);
        current = nextfd;
        if (j == std::string::npos) break;
        i = j + 1;
    }
    *out_fd = current;
    return 0;
}

static int create_parent_and_open_file(int basefd, const std::string& relpath, int* out_fd) {
    size_t slash = relpath.rfind('/');
    std::string parent = (slash == std::string::npos) ? std::string() : relpath.substr(0, slash);
    std::string fname = (slash == std::string::npos) ? relpath : relpath.substr(slash + 1);
    if (fname.empty() || fname == "." || fname == "..") {
        errno = EINVAL;
        return -1;
    }
    int parentfd = -1;
    if (mkdirs_at(basefd, parent, &parentfd) != 0) return -1;
    int fd = openat(parentfd, fname.c_str(),
                    O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    int saved = errno;
    close(parentfd);
    errno = saved;
    if (fd < 0) return -1;
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        int e = errno;
        close(fd);
        errno = e ? e : EINVAL;
        return -1;
    }
    *out_fd = fd;
    return 0;
}

static int extract_tar_secure_from_fd(int tarfd, int destfd) {
    unsigned char header[TAR_BLOCK];
    while (1) {
        ssize_t r = read_full(tarfd, header, TAR_BLOCK);
        if (r == 0) break;
        if (r < 0) return -1;
        if ((size_t)r < TAR_BLOCK) { errno = EINVAL; return -1; }
        if (is_zero_block(header)) {
            // read possible second zero block
            r = read_full(tarfd, header, TAR_BLOCK);
            if (r < 0) return -1;
            if ((size_t)r < TAR_BLOCK) break;
            if (is_zero_block(header)) break;
            errno = EINVAL;
            return -1;
        }
        std::string name = read_tar_string(header, 0, 100);
        std::string prefix = read_tar_string(header, 345, 155);
        std::string path = prefix.empty() ? name : (prefix + "/" + name);
        if (!is_safe_relpath(path)) {
            // skip entry
            off_t size = 0;
            if (!parse_octal(header, 124, 12, size) || size < 0) return -1;
            off_t skip = size + ((TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK);
            while (skip > 0) {
                char buf[4096];
                ssize_t toread = (ssize_t)((skip > (off_t)sizeof(buf)) ? sizeof(buf) : skip);
                ssize_t nr = read(tarfd, buf, (size_t)toread);
                if (nr <= 0) return -1;
                skip -= nr;
            }
            continue;
        }
        unsigned char typeflag = header[156];
        off_t fsize = 0;
        if (!parse_octal(header, 124, 12, fsize) || fsize < 0 || fsize > MAX_FILE_SIZE) {
            errno = EINVAL;
            return -1;
        }
        if (typeflag == '5') {
            int dirfd = -1;
            if (mkdirs_at(destfd, path, &dirfd) != 0) return -1;
            close(dirfd);
        } else if (typeflag == '0' || typeflag == 0) {
            int outfd = -1;
            if (create_parent_and_open_file(destfd, path, &outfd) != 0) return -1;
            off_t remaining = fsize;
            char buf[8192];
            while (remaining > 0) {
                ssize_t toread = (ssize_t)((remaining > (off_t)sizeof(buf)) ? sizeof(buf) : remaining);
                ssize_t nr = read(tarfd, buf, (size_t)toread);
                if (nr <= 0) { close(outfd); return -1; }
                ssize_t nw = 0;
                char* p = buf;
                while (nw < nr) {
                    ssize_t w = write(outfd, p + nw, (size_t)(nr - nw));
                    if (w < 0) {
                        if (errno == EINTR) continue;
                        close(outfd);
                        return -1;
                    }
                    nw += w;
                }
                remaining -= nr;
            }
            fsync(outfd);
            close(outfd);
            // skip padding
            off_t pad = (TAR_BLOCK - (fsize % TAR_BLOCK)) % TAR_BLOCK;
            if (pad > 0) {
                char padbuf[512];
                if (read_full(tarfd, padbuf, (size_t)pad) < (ssize_t)pad) return -1;
            }
        } else {
            // skip unsupported
            off_t skip = fsize + ((TAR_BLOCK - (fsize % TAR_BLOCK)) % TAR_BLOCK);
            while (skip > 0) {
                char buf[4096];
                ssize_t toread = (ssize_t)((skip > (off_t)sizeof(buf)) ? sizeof(buf) : skip);
                ssize_t nr = read(tarfd, buf, (size_t)toread);
                if (nr <= 0) return -1;
                skip -= nr;
            }
        }
    }
    fsync(destfd);
    return 0;
}

int extract_tar_secure(const char* archive_path, const char* dest_dir) {
    if (!archive_path || !dest_dir) return -1;
    int aflags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
    int tarfd = open(archive_path, aflags);
    if (tarfd < 0) return -1;
    struct stat st;
    if (fstat(tarfd, &st) != 0 || !S_ISREG(st.st_mode)) {
        int e = errno;
        close(tarfd);
        errno = e ? e : EINVAL;
        return -1;
    }
    // Prepare destination directory
    mkdir(dest_dir, 0700);
    int destfd = open(dest_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (destfd < 0) {
        int e = errno;
        close(tarfd);
        errno = e;
        return -1;
    }
    int rc = extract_tar_secure_from_fd(tarfd, destfd);
    int e = errno;
    close(tarfd);
    fsync(destfd);
    close(destfd);
    errno = e;
    return rc;
}

int main() {
    const char* tests[5] = {
        "archive.zip",
        "./archive.zip",
        "/does/not/exist/archive.zip",
        "archive.zip",
        "archive.zip"
    };
    for (int i = 0; i < 5; ++i) {
        int rc = extract_tar_secure(tests[i], "/tmp/unpack");
        if (rc == 0) {
            printf("Test %d: OK\n", i + 1);
        } else {
            printf("Test %d: %s\n", i + 1, strerror(errno));
        }
    }
    return 0;
}