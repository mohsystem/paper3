#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Secure random bytes via /dev/urandom
static bool secure_random_bytes(unsigned char* buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return false;
    size_t off = 0;
    while (off < len) {
        ssize_t n = read(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (n == 0) {
            close(fd);
            return false;
        }
        off += (size_t)n;
    }
    close(fd);
    return true;
}

static bool is_valid_relname(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (char c : name) {
        if (!( (c >= 'a' && c <= 'z') ||
               (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') ||
               c == '.' || c == '-' || c == '_' )) {
            return false;
        }
    }
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) return false;
    return true;
}

std::string write_secret_file(const std::string& base_dir, const std::string& filename) {
    if (!is_valid_relname(filename)) {
        return std::string();
    }

    // Secret info
    const char* secret = "TOP-SECRET: Highly confidential data. Do not disclose.";
    size_t secret_len = std::strlen(secret);

    int dfd = open(base_dir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) {
        return std::string();
    }

    struct stat dst;
    if (fstat(dfd, &dst) != 0 || !S_ISDIR(dst.st_mode)) {
        close(dfd);
        return std::string();
    }

    // Create random temp filename
    unsigned char rnd[12];
    if (!secure_random_bytes(rnd, sizeof(rnd))) {
        close(dfd);
        return std::string();
    }
    char hex[25] = {0};
    for (size_t i = 0; i < sizeof(rnd); ++i) {
        std::snprintf(&hex[i*2], 3, "%02x", rnd[i]);
    }
    std::string tmpname = std::string(".tmp_") + hex + ".part";

    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int tfd = openat(dfd, tmpname.c_str(), flags, 0600);
    if (tfd < 0) {
        close(dfd);
        return std::string();
    }

    struct stat st;
    if (fstat(tfd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(tfd);
        close(dfd);
        return std::string();
    }

    // Write secret
    ssize_t written = 0;
    const char* p = secret;
    size_t left = secret_len;
    while (left > 0) {
        ssize_t n = write(tfd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(tfd);
            close(dfd);
            // Cleanup temp
            unlinkat(dfd, tmpname.c_str(), 0);
            return std::string();
        }
        p += n;
        left -= (size_t)n;
    }

    // Flush file content
    if (fsync(tfd) != 0) {
        close(tfd);
        close(dfd);
        unlinkat(dfd, tmpname.c_str(), 0);
        return std::string();
    }

    // Set read-only for owner
    if (fchmod(tfd, 0400) != 0) {
        close(tfd);
        close(dfd);
        unlinkat(dfd, tmpname.c_str(), 0);
        return std::string();
    }

    // Close temp file before rename
    if (close(tfd) != 0) {
        close(dfd);
        unlinkat(dfd, tmpname.c_str(), 0);
        return std::string();
    }

    // Rename into place atomically
    if (renameat(dfd, tmpname.c_str(), dfd, filename.c_str()) != 0) {
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }

    // Fsync directory to persist rename
    (void)fsync(dfd);
    close(dfd);

    // Construct full path
    std::string full = base_dir;
    if (!full.empty() && full.back() != '/')
        full.push_back('/');
    full += filename;
    return full;
}

int main() {
    // Create a temporary base directory
    char tmpl[] = "/tmp/safe_base_XXXXXX";
    char* d = mkdtemp(tmpl);
    if (d == nullptr) {
        std::cout << "Operation failed" << std::endl;
        return 0;
    }
    std::string base(d);

    std::vector<std::string> names = {
        "secret1.txt",
        "secret2.txt",
        "alpha-3.log",
        "note_4.dat",
        "report5"
    };

    for (const auto& n : names) {
        std::string path = write_secret_file(base, n);
        if (path.empty()) {
            std::cout << "Operation failed" << std::endl;
        } else {
            std::cout << path << std::endl;
        }
    }
    return 0;
}