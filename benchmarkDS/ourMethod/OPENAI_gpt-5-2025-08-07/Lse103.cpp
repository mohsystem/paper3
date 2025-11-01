#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const size_t MAX_NAME_LENGTH = 255;
static const off_t SIZE_LIMIT = 10 * 1024 * 1024; // 10 MiB

static bool is_simple_filename(const std::string &name) {
    if (name.empty() || name.size() > MAX_NAME_LENGTH) return false;
    if (name == "." || name == "..") return false;
    if (name.find('/') != std::string::npos) return false;
    if (name.find('\\') != std::string::npos) return false;
    if (name.find('\0') != std::string::npos) return false;
    return true;
}

static int open_base_dir(const std::string &base_dir) {
    if (base_dir.empty()) {
        errno = EINVAL;
        return -1;
    }
    int flags = O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW;
    int dfd = open(base_dir.c_str(), flags);
    if (dfd < 0) return -1;
    struct stat st;
    if (fstat(dfd, &st) != 0) {
        int e = errno;
        close(dfd);
        errno = e;
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        close(dfd);
        errno = ENOTDIR;
        return -1;
    }
    return dfd;
}

static ssize_t write_all(int fd, const void *buf, size_t len) {
    const unsigned char *p = static_cast<const unsigned char *>(buf);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, p + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += static_cast<size_t>(n);
    }
    if (fsync(fd) != 0) return -1;
    return static_cast<ssize_t>(off);
}

// Returns bytes written on success, -1 on failure.
int secure_create_new_file(const std::string &base_dir, const std::string &filename, const std::string &content) {
    if (!is_simple_filename(filename)) { errno = EINVAL; return -1; }
    int dfd = open_base_dir(base_dir);
    if (dfd < 0) return -1;

    int flags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW;
    int fd = openat(dfd, filename.c_str(), flags, 0600);
    if (fd < 0) { close(dfd); return -1; }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno; close(fd); close(dfd); errno = e; return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd); close(dfd); errno = EINVAL; return -1;
    }
    if (content.size() > static_cast<size_t>(SIZE_LIMIT)) {
        close(fd); close(dfd); errno = EFBIG; return -1;
    }

    ssize_t wrote = write_all(fd, content.data(), content.size());
    int e = errno;
    close(fd);
    // best-effort sync directory
    (void)fsync(dfd);
    close(dfd);
    if (wrote < 0) { errno = e; return -1; }
    return static_cast<int>(wrote);
}

// Returns bytes written on success, -1 on failure.
int secure_truncate_and_write_file(const std::string &base_dir, const std::string &filename, const std::string &content) {
    if (!is_simple_filename(filename)) { errno = EINVAL; return -1; }
    int dfd = open_base_dir(base_dir);
    if (dfd < 0) return -1;

    int flags = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW;
    int fd = openat(dfd, filename.c_str(), flags, 0600);
    if (fd < 0) { close(dfd); return -1; }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno; close(fd); close(dfd); errno = e; return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd); close(dfd); errno = EINVAL; return -1;
    }
    if (content.size() > static_cast<size_t>(SIZE_LIMIT)) {
        close(fd); close(dfd); errno = EFBIG; return -1;
    }

    ssize_t wrote = write_all(fd, content.data(), content.size());
    int e = errno;
    close(fd);
    // best-effort sync directory
    (void)fsync(dfd);
    close(dfd);
    if (wrote < 0) { errno = e; return -1; }
    return static_cast<int>(wrote);
}

int main() {
    // Create a temporary base directory
    char tmpl[] = "/tmp/secure_open_demo_XXXXXX";
    char *base = mkdtemp(tmpl);
    if (!base) {
        std::cout << "fatal\n";
        return 0;
    }
    std::string baseDir(base);

    // 1) Create new file (success)
    int r1 = secure_create_new_file(baseDir, "a.txt", "hello");
    std::cout << "Test1 create_new a.txt bytes=" << (r1 >= 0 ? r1 : -1) << "\n";

    // 2) Create same file again (should fail)
    int r2 = secure_create_new_file(baseDir, "a.txt", "x");
    if (r2 >= 0) std::cout << "Test2 unexpected success\n";
    else std::cout << "Test2 expected failure\n";

    // 3) Truncate and write existing file
    int r3 = secure_truncate_and_write_file(baseDir, "a.txt", "world");
    std::cout << "Test3 truncate a.txt bytes=" << (r3 >= 0 ? r3 : -1) << "\n";

    // 4) Create new different file
    int r4 = secure_create_new_file(baseDir, "b.txt", "data");
    std::cout << "Test4 create_new b.txt bytes=" << (r4 >= 0 ? r4 : -1) << "\n";

    // 5) Invalid path traversal attempt
    int r5 = secure_create_new_file(baseDir, "../evil.txt", "bad");
    if (r5 >= 0) std::cout << "Test5 unexpected success\n";
    else std::cout << "Test5 expected failure\n";

    return 0;
}