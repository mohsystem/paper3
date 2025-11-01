#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>
#include <ctime>
#include <sys/types.h>

static int safe_write_all(int fd, const void* buf, size_t len) {
    const char* p = static_cast<const char*>(buf);
    size_t total = 0;
    while (total < len) {
        ssize_t n = ::write(fd, p + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1;
        total += static_cast<size_t>(n);
    }
    return 0;
}

// O_CREAT | O_WRONLY | O_TRUNC
int create_or_truncate_write(const std::string& path, const std::string& data) {
    int flags = O_CREAT | O_WRONLY | O_TRUNC;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    mode_t mode = 0600;
    int fd = ::open(path.c_str(), flags, mode);
    if (fd < 0) {
        return -1;
    }
    int rc = safe_write_all(fd, data.data(), data.size());
    if (rc == 0) {
        (void)::fsync(fd);
    }
    int err = errno;
    ::close(fd);
    if (rc != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

// O_CREAT | O_WRONLY | O_EXCL
int create_exclusive_write(const std::string& path, const std::string& data) {
    int flags = O_CREAT | O_WRONLY | O_EXCL;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    mode_t mode = 0600;
    int fd = ::open(path.c_str(), flags, mode);
    if (fd < 0) {
        if (errno == EEXIST) return 1;
        return -1;
    }
    int rc = safe_write_all(fd, data.data(), data.size());
    if (rc == 0) {
        (void)::fsync(fd);
    }
    int err = errno;
    ::close(fd);
    if (rc != 0) {
        errno = err;
        return -1;
    }
    return 0;
}

static std::string read_all(const std::string& path) {
    std::string out;
    int fd = ::open(path.c_str(), O_RDONLY
#ifdef O_CLOEXEC
    | O_CLOEXEC
#endif
    );
    if (fd < 0) return out;
    char buf[4096];
    ssize_t n;
    while ((n = ::read(fd, buf, sizeof(buf))) > 0) {
        out.append(buf, buf + n);
    }
    ::close(fd);
    return out;
}

int main() {
    pid_t pid = getpid();
    long long t = static_cast<long long>(time(nullptr));
    std::string dir = "/tmp";
    std::string pathA = dir + "/lse103_cpp_A_" + std::to_string(pid) + "_" + std::to_string(t) + ".txt";
    std::string pathB = dir + "/lse103_cpp_B_" + std::to_string(pid) + "_" + std::to_string(t) + ".txt";

    int r1 = create_exclusive_write(pathA, "hello exclusive A");
    std::cout << "T1 ex-create A: " << r1 << "\n";

    int r2 = create_exclusive_write(pathA, "should fail");
    std::cout << "T2 ex-create A again: " << r2 << "\n";

    int r3 = create_or_truncate_write(pathA, "truncated A");
    std::cout << "T3 truncate-write A: " << r3 << " len=" << read_all(pathA).size() << "\n";

    int r4 = create_or_truncate_write(pathB, "new B");
    std::cout << "T4 create-or-truncate B: " << r4 << " len=" << read_all(pathB).size() << "\n";

    int r5 = create_exclusive_write(pathB, "should fail B");
    std::cout << "T5 ex-create B again: " << r5 << "\n";

    ::unlink(pathA.c_str());
    ::unlink(pathB.c_str());
    return 0;
}