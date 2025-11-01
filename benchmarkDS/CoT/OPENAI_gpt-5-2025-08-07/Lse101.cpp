#include <iostream>
#include <string>

#if defined(_WIN32)
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define OPEN_FN _open
  #define WRITE_FN _write
  #define CLOSE_FN _close
  #define SYNC_FN _commit
  #define MODE_RW (_S_IREAD | _S_IWRITE)
  #ifndef O_BINARY
  #define O_BINARY 0
  #endif
#else
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/stat.h>
  #define OPEN_FN open
  #define WRITE_FN write
  #define CLOSE_FN close
  #define SYNC_FN fsync
  #define MODE_RW 0600
#endif

// Step 1-2: Implement functionality and apply secure defaults.
bool secure_write(const std::string& filename, const std::string& content) {
#if defined(_WIN32)
    int flags = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
#else
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#endif
    int fd = OPEN_FN(filename.c_str(), flags, MODE_RW);
    if (fd < 0) return false;

#ifndef _WIN32
    // Step 4: Ensure restrictive permissions on existing file (best-effort).
    (void)fchmod(fd, MODE_RW);
#endif

    const char* buf = content.c_str();
    size_t total = content.size();
    size_t written = 0;
    while (written < total) {
        ssize_t n = WRITE_FN(fd, buf + written, static_cast<unsigned int>(total - written));
        if (n <= 0) {
            CLOSE_FN(fd);
            return false;
        }
        written += static_cast<size_t>(n);
    }
    (void)SYNC_FN(fd);
    CLOSE_FN(fd);
    return true;
}

int main() {
    std::cout << std::boolalpha;
    std::cout << secure_write("important_config", "important_config") << "\n";
    std::cout << secure_write("important_config", "important_config") << "\n";
    std::cout << secure_write("./important_config", "important_config") << "\n";
    std::cout << secure_write("important_config", "important_config") << "\n";
    std::cout << secure_write("important_config", "important_config") << "\n";
    return 0;
}