#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

static char* mode_to_symbolic(unsigned int mode) {
    char* s = (char*)malloc(11);
    if (!s) return NULL;
    s[0] = '-';
    s[1] = (mode & 0400) ? 'r' : '-';
    s[2] = (mode & 0200) ? 'w' : '-';
    s[3] = (mode & 0100) ? 'x' : '-';
    s[4] = (mode & 0040) ? 'r' : '-';
    s[5] = (mode & 0020) ? 'w' : '-';
    s[6] = (mode & 0010) ? 'x' : '-';
    s[7] = (mode & 0004) ? 'r' : '-';
    s[8] = (mode & 0002) ? 'w' : '-';
    s[9] = (mode & 0001) ? 'x' : '-';
    s[10] = '\0';
    return s;
}

char* create_and_restrict(const char* path, const char* data) {
#if defined(_WIN32)
    int fd = _open(path, _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY, _S_IREAD | _S_IWRITE);
    if (fd < 0) {
        char* msg = (char*)malloc(256);
        if (!msg) return NULL;
        snprintf(msg, 256, "ERROR: cannot open");
        printf("%s\n", msg);
        return msg;
    }
    (void)_write(fd, data, (unsigned)strlen(data));
    _close(fd);
    _chmod(path, _S_IREAD | _S_IWRITE);
    char* perm = (char*)malloc(11);
    if (!perm) return NULL;
    strcpy(perm, "-rw-------");
    printf("%s\n", perm);
    return perm;
#else
    struct stat lst;
    if (lstat(path, &lst) == 0 && S_ISLNK(lst.st_mode)) {
        char* msg = (char*)malloc(64);
        if (!msg) return NULL;
        strcpy(msg, "ERROR: Refusing to operate on symlink");
        printf("%s\n", msg);
        return msg;
    }

    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif

    int fd = open(path, flags, 0600);
    if (fd < 0) {
        char* msg = (char*)malloc(64);
        if (!msg) return NULL;
        strcpy(msg, "ERROR: open failed");
        printf("%s\n", msg);
        return msg;
    }

    size_t to_write = strlen(data);
    size_t written = 0;
    while (written < to_write) {
        ssize_t n = write(fd, data + written, to_write - written);
        if (n < 0) {
            char* msg = (char*)malloc(64);
            if (!msg) { close(fd); return NULL; }
            strcpy(msg, "ERROR: write failed");
            printf("%s\n", msg);
            close(fd);
            return msg;
        }
        written += (size_t)n;
    }
    fsync(fd);
    close(fd);

    chmod(path, 0600);

    struct stat st;
    if (lstat(path, &st) != 0) {
        char* msg = (char*)malloc(64);
        if (!msg) return NULL;
        strcpy(msg, "ERROR: stat failed");
        printf("%s\n", msg);
        return msg;
    }

    char* perm = mode_to_symbolic(st.st_mode);
    if (!perm) return NULL;
    printf("%s\n", perm);
    return perm;
#endif
}

int main(void) {
    const char* secret = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    char* p1 = create_and_restrict("secret.txt", secret); if (p1) free(p1);
    char* p2 = create_and_restrict("secret1.txt", secret); if (p2) free(p2);
    char* p3 = create_and_restrict("secret2.txt", secret); if (p3) free(p3);
    char* p4 = create_and_restrict("secret3.txt", secret); if (p4) free(p4);
    char* p5 = create_and_restrict("secret4.txt", secret); if (p5) free(p5);
    return 0;
}