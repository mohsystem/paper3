#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

// Chain-of-Through Step 1: Problem understanding & function signature
// Open/create file with 0644 and write content, return bytes written or -1 on error.
long long secure_open_write(const std::string& filename, const std::string& content) {
    if (filename.empty()) {
        return -1;
    }

    // Chain-of-Through Step 2: Security requirements
    int flags = O_RDWR | O_CREAT;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    mode_t mode = static_cast<mode_t>(0644);

    // Chain-of-Through Step 3: Secure coding generation
    int fd = open(filename.c_str(), flags, mode);
    if (fd < 0) {
        return -1;
    }

    // Ensure it's a regular file
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return -1;
    }

    // Force exact permissions regardless of umask
    if (fchmod(fd, mode) != 0) {
        // Continue even if fchmod fails, but could be considered error
    }

    const char* buf = content.c_str();
    size_t len = content.size();
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        off += static_cast<size_t>(n);
    }

    // Flush to disk
    (void)fsync(fd);
    close(fd);
    return static_cast<long long>(off);
}

// Chain-of-Through Step 4 & 5 are reflected in the careful checks above.

int main() {
    const std::string content = "important_config";
    const std::string files[5] = {
        "important_config",
        "important_config_1",
        "important_config_2",
        "important_config", // re-open existing
        "important_config_readwrite"
    };

    for (int i = 0; i < 5; ++i) {
        long long written = secure_open_write(files[i], content);
        if (written >= 0) {
            std::cout << "Test " << (i + 1) << ": Wrote " << written << " bytes to " << files[i] << "\n";
        } else {
            std::cout << "Test " << (i + 1) << " failed for " << files[i] << ": " << std::strerror(errno) << "\n";
        }
    }
    return 0;
}