#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cctype>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const size_t MAX_SIZE = 10 * 1024 * 1024;

static bool valid_filename(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    for (unsigned char c : name) {
        if (!(std::isalnum(c) || c == '_' || c == '-' || c == '.')) return false;
    }
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) return false;
    return true;
}

int upload_file(const std::string& filename, const std::vector<unsigned char>& data, std::string& saved_path) {
    if (!valid_filename(filename)) return -1;
    if (data.size() > MAX_SIZE) return -2;

    int basefd = -1, updfd = -1, tmpfd = -1;
    std::string tmpname;

    basefd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (basefd < 0) return -3;

    // Ensure uploads directory exists
    if (mkdirat(basefd, "uploads", 0700) < 0 && errno != EEXIST) {
        close(basefd);
        return -4;
    }

    updfd = openat(basefd, "uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (updfd < 0) {
        close(basefd);
        return -5;
    }

    // Create unique temp name
    std::random_device rd;
    std::mt19937_64 gen(rd());
    unsigned long long r = gen();
    tmpname = ".tmp-" + std::to_string(getpid()) + "-" + std::to_string(r);

    tmpfd = openat(updfd, tmpname.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (tmpfd < 0) {
        close(updfd);
        close(basefd);
        return -6;
    }

    // Validate it's a regular file
    struct stat st;
    if (fstat(tmpfd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(tmpfd);
        unlinkat(updfd, tmpname.c_str(), 0);
        close(updfd);
        close(basefd);
        return -7;
    }

    // Write data
    size_t offset = 0;
    while (offset < data.size()) {
        ssize_t n = write(tmpfd, data.data() + offset, data.size() - offset);
        if (n < 0) {
            int e = errno;
            close(tmpfd);
            unlinkat(updfd, tmpname.c_str(), 0);
            close(updfd);
            close(basefd);
            errno = e;
            return -8;
        }
        offset += static_cast<size_t>(n);
    }

    if (fsync(tmpfd) != 0) {
        int e = errno;
        close(tmpfd);
        unlinkat(updfd, tmpname.c_str(), 0);
        close(updfd);
        close(basefd);
        errno = e;
        return -9;
    }

    // Attempt to link into destination (no overwrite)
    int link_rc = linkat(updfd, tmpname.c_str(), updfd, filename.c_str(), 0);
    int link_errno = errno;

    // Remove temp name
    int tmp_close_rc = close(tmpfd);
    (void)tmp_close_rc;
    unlinkat(updfd, tmpname.c_str(), 0);

    if (link_rc != 0) {
        close(updfd);
        close(basefd);
        errno = link_errno;
        return -10;
    }

    // Optionally fsync directory
    (void)fsync(updfd);

    saved_path = std::string("uploads/") + filename;

    close(updfd);
    close(basefd);
    return 0;
}

static void run_test(const std::string& name, const std::vector<unsigned char>& data) {
    std::string out;
    int rc = upload_file(name, data, out);
    if (rc == 0) {
        std::cout << "OK: " << name << " -> " << out << "\n";
    } else {
        std::cout << "ERR: " << name << " -> rc=" << rc << " errno=" << errno << "\n";
    }
}

int main() {
    run_test("a.txt", std::vector<unsigned char>{'h','e','l','l','o'}); // success
    run_test("b.png", std::vector<unsigned char>{1,2,3});               // success
    run_test("../bad.txt", std::vector<unsigned char>{'o','o','p','s'}); // traversal -> error
    run_test("bad/inner.txt", std::vector<unsigned char>{'o','o','p','s'}); // separator -> error
    run_test("big.bin", std::vector<unsigned char>(MAX_SIZE + 1, 0));   // too large -> error
    return 0;
}