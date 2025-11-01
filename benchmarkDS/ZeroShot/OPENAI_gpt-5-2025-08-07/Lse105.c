#include <stdio.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

static int is_simple_filename(const char* name) {
    if (name == NULL || *name == '\0') return 0;
#if defined(_WIN32)
    for (const char* p = name; *p; ++p) {
        if (*p == '\\' || *p == '/' || *p == ':') return 0;
    }
#else
    for (const char* p = name; *p; ++p) {
        if (*p == '/') return 0;
    }
#endif
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) return 0;
    return 1;
}

int create_secret_file(const char* filename, const char* content) {
    if (!is_simple_filename(filename) || content == NULL) return -1;

#if defined(_WIN32)
    int fd = -1;
    errno_t err = _sopen_s(&fd, filename, _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _SH_DENYRW, _S_IREAD | _S_IWRITE);
    if (err != 0 || fd < 0) {
        return -1;
    }
    size_t len = strlen(content);
    size_t total = 0;
    while (total < len) {
        int written = _write(fd, content + total, (unsigned int)(len - total));
        if (written <= 0) {
            _close(fd);
            return -1;
        }
        total += (size_t)written;
    }
    _close(fd);
    _chmod(filename, _S_IREAD | _S_IWRITE);
    return 0;
#else
    int flags = O_WRONLY | O_CREAT | O_EXCL;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    mode_t mode = S_IRUSR | S_IWUSR; // 0600
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        return -1;
    }
    const char* data = content;
    size_t len = strlen(data);
    size_t total = 0;
    while (total < len) {
        ssize_t w = write(fd, data + total, len - total);
        if (w <= 0) {
            close(fd);
            return -1;
        }
        total += (size_t)w;
    }
    fchmod(fd, mode);
    close(fd);
    chmod(filename, mode);
    return 0;
#endif
}

int main(void) {
    const char* secret = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    printf("%d\n", create_secret_file("secret.txt", secret) == 0 ? 1 : 0);
    printf("%d\n", create_secret_file("secret1.txt", secret) == 0 ? 1 : 0);
    printf("%d\n", create_secret_file("secret2.txt", secret) == 0 ? 1 : 0);
    printf("%d\n", create_secret_file("secret3.txt", secret) == 0 ? 1 : 0);
    printf("%d\n", create_secret_file("secret4.txt", secret) == 0 ? 1 : 0);
    return 0;
}