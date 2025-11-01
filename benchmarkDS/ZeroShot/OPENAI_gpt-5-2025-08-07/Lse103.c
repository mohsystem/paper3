#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

bool secure_open_write(const char* path, bool create, bool trunc, bool wr_only, bool excl) {
    if (excl && !create) {
        return false;
    }

    int flags = 0;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    flags |= wr_only ? O_WRONLY : O_RDWR;

    if (create) flags |= O_CREAT;
    if (excl)   flags |= O_EXCL;
    if (trunc)  flags |= O_TRUNC;

    mode_t mode = S_IRUSR | S_IWUSR; // 0600
    int fd = open(path, flags, mode);
    if (fd < 0) {
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

int main(void) {
    const char* dir = "lse103_tmp_c";
    mkdir(dir, 0700);

    char f1[256];
    char f2[256];
    snprintf(f1, sizeof(f1), "%s/%s", dir, "a.txt");
    snprintf(f2, sizeof(f2), "%s/%s", dir, "nonexist.txt");

    printf("%s\n", secure_open_write(f1, true,  false, true, true)  ? "true" : "false");
    printf("%s\n", secure_open_write(f1, true,  false, true, true)  ? "true" : "false");
    printf("%s\n", secure_open_write(f1, false, true,  true, false) ? "true" : "false");
    printf("%s\n", secure_open_write(f1, false, false, true, false) ? "true" : "false");
    printf("%s\n", secure_open_write(f2, false, false, true, false) ? "true" : "false");

    return 0;
}