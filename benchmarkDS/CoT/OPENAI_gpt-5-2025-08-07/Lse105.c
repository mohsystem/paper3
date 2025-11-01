/*
Chain-of-Through:
1) Problem: Create/overwrite secret.txt; set restrictive file permissions.
2) Security: Create with restrictive perms (0600 POSIX, owner R/W on Windows) to avoid exposure.
3) Generation: Use open/_sopen_s; write; fsync/_commit; chmod/_chmod to enforce perms.
4) Review: Handle partial writes and errors; minimal surface.
5) Output: Returns int success(1)/failure(0); main runs 5 tests.
*/

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <share.h>
#else
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <unistd.h>
#endif

int create_secret_file(const char* filename, const char* content) {
#ifdef _WIN32
    int fd = -1;
    errno_t err = _sopen_s(&fd, filename,
                           _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY,
                           _SH_DENYNO,
                           _S_IREAD | _S_IWRITE);
    if (err != 0 || fd < 0) {
        return 0;
    }
    size_t len = strlen(content);
    size_t total = 0;
    while (total < len) {
        int wrote = _write(fd, content + total, (unsigned int)(len - total));
        if (wrote <= 0) {
            _close(fd);
            return 0;
        }
        total += (size_t)wrote;
    }
    _commit(fd);
    _close(fd);
    _chmod(filename, _S_IREAD | _S_IWRITE);
    return 1;
#else
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        return 0;
    }
    size_t len = strlen(content);
    size_t total = 0;
    while (total < len) {
        ssize_t wrote = write(fd, content + total, len - total);
        if (wrote <= 0) {
            close(fd);
            return 0;
        }
        total += (size_t)wrote;
    }
    fsync(fd);
    fchmod(fd, 0600);
    close(fd);
    return 1;
#endif
}

int main(void) {
    const char* filename = "secret.txt";
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    for (int i = 1; i <= 5; ++i) {
        int ok = create_secret_file(filename, content);
        printf("Test %d: %s\n", i, ok ? "true" : "false");
    }
    return 0;
}