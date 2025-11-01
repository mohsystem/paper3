#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <random>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100
#endif

static std::string mode_to_string(mode_t mode) {
    std::string s(10, '-');
    s[0] = '-';
    s[1] = (mode & S_IRUSR) ? 'r' : '-';
    s[2] = (mode & S_IWUSR) ? 'w' : '-';
    s[3] = (mode & S_IXUSR) ? 'x' : '-';
    s[4] = (mode & S_IRGRP) ? 'r' : '-';
    s[5] = (mode & S_IWGRP) ? 'w' : '-';
    s[6] = (mode & S_IXGRP) ? 'x' : '-';
    s[7] = (mode & S_IROTH) ? 'r' : '-';
    s[8] = (mode & S_IWOTH) ? 'w' : '-';
    s[9] = (mode & S_IXOTH) ? 'x' : '-';
    return s;
}

static bool write_all(int fd, const char* data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = ::write(fd, data + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += static_cast<size_t>(n);
    }
    return true;
}

std::string create_secret_and_print_perms(const std::string& base_dir) {
    const char* CONTENT = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    const char* TARGET = "secret.txt";

    int dirfd = ::open(base_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) return "Error";

    struct stat dst;
    if (fstat(dirfd, &dst) != 0 || !S_ISDIR(dst.st_mode)) {
        close(dirfd);
        return "Error";
    }

    // Create temp file name and open securely
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;

    int tmpfd = -1;
    std::string tmpname;
    for (int i = 0; i < 100; ++i) {
        std::ostringstream oss;
        oss << ".secret_tmp_" << std::hex << dist(gen);
        tmpname = oss.str();
        tmpfd = ::openat(dirfd, tmpname.c_str(),
                         O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW,
                         0600);
        if (tmpfd >= 0) break;
        if (errno != EEXIST) continue;
    }
    if (tmpfd < 0) {
        close(dirfd);
        return "Error";
    }

    bool ok = write_all(tmpfd, CONTENT, std::strlen(CONTENT));
    if (ok) ok = (fsync(tmpfd) == 0);
    if (ok) ok = (fchmod(tmpfd, 0600) == 0);
    int saved = errno;
    close(tmpfd);
    errno = saved;
    if (!ok) {
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        return "Error";
    }

    // fsync directory to persist temp file
    (void)fsync(dirfd);

    // Atomically move into place
    if (renameat(dirfd, tmpname.c_str(), dirfd, TARGET) != 0) {
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        return "Error";
    }

    // Ensure restrictive permissions
    int tfd = ::openat(dirfd, TARGET, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (tfd >= 0) {
        (void)fchmod(tfd, 0600);
        close(tfd);
    }

    struct stat st;
    if (fstatat(dirfd, TARGET, &st, AT_SYMLINK_NOFOLLOW) != 0) {
        close(dirfd);
        return "Error";
    }
    close(dirfd);

    std::ostringstream out;
    out << "Permissions: " << mode_to_string(st.st_mode) << " (" << std::oct << std::showbase << (st.st_mode & 07777) << ")";
    return out.str();
}

static std::string mktempdir() {
    std::string templ = "/tmp/safe_dir_XXXXXX";
    std::vector<char> buf(templ.begin(), templ.end());
    buf.push_back('\0');
    char* d = ::mkdtemp(buf.data());
    if (!d) return "/tmp";
    return std::string(d);
}

int main() {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        std::string d = mktempdir();
        std::string r = create_secret_and_print_perms(d);
        std::cout << "Base: " << d << " -> " << r << std::endl;
    }
    return 0;
}