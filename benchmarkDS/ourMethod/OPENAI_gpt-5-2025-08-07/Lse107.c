#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#define AT_SYMLINK_NOFOLLOW 0x100
#endif

static void mode_to_string(mode_t mode, char out[11]) {
    out[0] = '-';
    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

static int write_all(int fd, const char* data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, data + off, len - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)n;
    }
    return 0;
}

char* create_secret_and_print_perms(const char* base_dir) {
    const char* CONTENT = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    const char* TARGET = "secret.txt";

    int dirfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) {
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }

    struct stat st_dir;
    if (fstat(dirfd, &st_dir) != 0 || !S_ISDIR(st_dir.st_mode)) {
        close(dirfd);
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }

    // Create temp file
    int tmpfd = -1;
    char tmpname[64];
    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    for (int i = 0; i < 100; ++i) {
        unsigned long r = ((unsigned long)rand() << 16) ^ (unsigned long)rand();
        snprintf(tmpname, sizeof(tmpname), ".secret_tmp_%08lx", r);
        tmpfd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (tmpfd >= 0) break;
        if (errno != EEXIST) continue;
    }
    if (tmpfd < 0) {
        close(dirfd);
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }

    int ok = 0;
    if (write_all(tmpfd, CONTENT, strlen(CONTENT)) == 0 &&
        fsync(tmpfd) == 0 &&
        fchmod(tmpfd, 0600) == 0) {
        ok = 1;
    }
    int saved = errno;
    close(tmpfd);
    errno = saved;
    if (!ok) {
        unlinkat(dirfd, tmpname, 0);
        close(dirfd);
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }

    // fsync directory
    (void)fsync(dirfd);

    // Move to target
    if (renameat(dirfd, tmpname, dirfd, TARGET) != 0) {
        unlinkat(dirfd, tmpname, 0);
        close(dirfd);
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }

    // Ensure restrictive perms
    int tfd = openat(dirfd, TARGET, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (tfd >= 0) {
        (void)fchmod(tfd, 0600);
        close(tfd);
    }

    struct stat st;
    if (fstatat(dirfd, TARGET, &st, AT_SYMLINK_NOFOLLOW) != 0) {
        close(dirfd);
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }
    close(dirfd);

    char modebuf[11];
    mode_to_string(st.st_mode, modebuf);
    char outbuf[64];
    snprintf(outbuf, sizeof(outbuf), "Permissions: %s (%04o)", modebuf, st.st_mode & 07777);
    size_t n = strlen(outbuf);
    char* ret = (char*)malloc(n + 1);
    if (!ret) {
        char* r = (char*)malloc(6);
        if (r) strcpy(r, "Error");
        return r;
    }
    memcpy(ret, outbuf, n + 1);
    return ret;
}

static char* mktempdir(void) {
    char* templ = strdup("/tmp/safe_dir_XXXXXX");
    if (!templ) return NULL;
    if (!mkdtemp(templ)) {
        free(templ);
        return NULL;
    }
    return templ;
}

int main(void) {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        char* d = mktempdir();
        if (!d) {
            puts("Error creating temp dir");
            continue;
        }
        char* r = create_secret_and_print_perms(d);
        printf("Base: %s -> %s\n", d, r ? r : "Error");
        free(r);
        free(d);
    }
    return 0;
}