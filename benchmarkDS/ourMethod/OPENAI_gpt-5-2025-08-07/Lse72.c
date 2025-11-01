#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/random.h>
#endif

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define MAX_SIZE (10 * 1024 * 1024)

static int valid_filename(const char *name) {
    if (name == NULL) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 255) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-' || c == '.')) {
            return 0;
        }
    }
    if (strchr(name, '/') || strchr(name, '\\')) return 0;
    return 1;
}

static void wipe(void *p, size_t n) {
#if defined(__STDC_LIB_EXT1__)
    memset_s(p, n, 0, n);
#else
    volatile unsigned char *vp = (volatile unsigned char *)p;
    while (n--) *vp++ = 0;
#endif
}

static int get_rand_u64(unsigned long long *out) {
    if (!out) return -1;
#ifdef __linux__
    if (getrandom(out, sizeof(*out), 0) == (ssize_t)sizeof(*out)) return 0;
#endif
    // Fallback: /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return -1;
    ssize_t n = read(fd, out, sizeof(*out));
    int e = errno;
    close(fd);
    errno = e;
    return (n == (ssize_t)sizeof(*out)) ? 0 : -1;
}

int upload_file(const char *filename, const unsigned char *data, size_t data_len,
                char *out_path, size_t out_path_sz) {
    if (!filename || !data || !out_path || out_path_sz == 0) return -100;
    if (!valid_filename(filename)) return -101;
    if (data_len > MAX_SIZE) return -102;

    int basefd = -1, updfd = -1, tmpfd = -1;
    char tmpname[128];
    int rc = 0;

    basefd = open(".", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (basefd < 0) { rc = -103; goto cleanup; }

    if (mkdirat(basefd, "uploads", 0700) < 0 && errno != EEXIST) { rc = -104; goto cleanup; }

    updfd = openat(basefd, "uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (updfd < 0) { rc = -105; goto cleanup; }

    unsigned long long rnd = 0;
    if (get_rand_u64(&rnd) != 0) rnd = (unsigned long long)getpid() ^ 0xA5A5A5A5ULL;

    int n = snprintf(tmpname, sizeof(tmpname), ".tmp-%ld-%llx", (long)getpid(), (unsigned long long)rnd);
    if (n <= 0 || (size_t)n >= sizeof(tmpname)) { rc = -106; goto cleanup; }

    tmpfd = openat(updfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (tmpfd < 0) { rc = -107; goto cleanup; }

    struct stat st;
    if (fstat(tmpfd, &st) != 0 || !S_ISREG(st.st_mode)) { rc = -108; goto cleanup; }

    size_t written = 0;
    while (written < data_len) {
        ssize_t w = write(tmpfd, data + written, data_len - written);
        if (w < 0) { rc = -109; goto cleanup; }
        written += (size_t)w;
    }

    if (fsync(tmpfd) != 0) { rc = -110; goto cleanup; }

    // Link into destination (no overwrite)
    if (linkat(updfd, tmpname, updfd, filename, 0) != 0) { rc = -111; goto cleanup; }

    // Remove temp name
    (void)unlinkat(updfd, tmpname, 0);

    // Optionally fsync directory
    (void)fsync(updfd);

    if (snprintf(out_path, out_path_sz, "uploads/%s", filename) >= (int)out_path_sz) {
        rc = -112; goto cleanup;
    }

cleanup:
    {
        int saved = errno;
        if (tmpfd >= 0) close(tmpfd);
        if (rc != 0 && updfd >= 0 && tmpname[0]) (void)unlinkat(updfd, tmpname, 0);
        if (updfd >= 0) close(updfd);
        if (basefd >= 0) close(basefd);
        errno = saved;
    }
    return rc;
}

static void run_test(const char *name, const unsigned char *data, size_t len) {
    char path[1024];
    int r = upload_file(name, data, len, path, sizeof(path));
    if (r == 0) {
        printf("OK: %s -> %s\n", name, path);
    } else {
        printf("ERR: %s -> rc=%d errno=%d\n", name, r, errno);
    }
}

int main(void) {
    const unsigned char a1[] = "hello";
    const unsigned char a2[] = {1,2,3};
    const unsigned char bad[] = "oops";
    unsigned char *big = (unsigned char*)malloc(MAX_SIZE + 1);
    if (!big) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    memset(big, 0, MAX_SIZE + 1);

    run_test("a.txt", a1, sizeof(a1) - 1);                 // success
    run_test("b.png", a2, sizeof(a2));                      // success
    run_test("../bad.txt", bad, sizeof(bad) - 1);           // traversal -> error
    run_test("bad/inner.txt", bad, sizeof(bad) - 1);        // separator -> error
    run_test("big.bin", big, MAX_SIZE + 1);                 // too large -> error

    wipe(big, MAX_SIZE + 1);
    free(big);
    return 0;
}