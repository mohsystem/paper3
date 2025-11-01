#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
bool secure_write(const char* filename, const char* content) {
    if (filename == NULL || content == NULL) return false;

#if defined(_WIN32)
    int flags = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
#else
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#endif

    int fd = OPEN_FN(filename, flags, MODE_RW);
    if (fd < 0) return false;

#ifndef _WIN32
    // Step 4: Ensure restrictive permissions on existing file (best-effort).
    (void)fchmod(fd, MODE_RW);
#endif

    size_t total = strlen(content);
    size_t written = 0;
    while (written < total) {
        int n = WRITE_FN(fd, content + written, (unsigned int)(total - written));
        if (n <= 0) {
            CLOSE_FN(fd);
            return false;
        }
        written += (size_t)n;
    }
    (void)SYNC_FN(fd);
    CLOSE_FN(fd);
    return true;
}

int main(void) {
    printf("%s\n", secure_write("important_config", "important_config") ? "true" : "false");
    printf("%s\n", secure_write("important_config", "important_config") ? "true" : "false");
    printf("%s\n", secure_write("./important_config", "important_config") ? "true" : "false");
    printf("%s\n", secure_write("important_config", "important_config") ? "true" : "false");
    printf("%s\n", secure_write("important_config", "important_config") ? "true" : "false");
    return 0;
}