#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MAX_SPAN 1000000000ULL

// Read a cryptographically secure 64-bit random value from /dev/urandom using openat to avoid symlink issues.
static int get_secure_u64(uint64_t *out) {
    if (out == NULL) {
        return -1;
    }
    int dirfd = -1;
    int fd = -1;
    struct stat st;
    ssize_t r;
    size_t got = 0;

    dirfd = open("/dev", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) goto fail;

    fd = openat(dirfd, "urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) goto fail;

    if (fstat(fd, &st) != 0) goto fail;
    if (!S_ISCHR(st.st_mode)) goto fail;

    unsigned char *p = (unsigned char*)out;
    while (got < sizeof(uint64_t)) {
        r = read(fd, p + got, sizeof(uint64_t) - got);
        if (r < 0) {
            if (errno == EINTR) continue;
            goto fail;
        }
        if (r == 0) goto fail;
        got += (size_t)r;
    }

    if (fd >= 0) close(fd);
    if (dirfd >= 0) close(dirfd);
    return 0;

fail:
    if (fd >= 0) close(fd);
    if (dirfd >= 0) close(dirfd);
    return -1;
}

// Generate a uniform random number in [0, n) using rejection sampling with a secure 64-bit source.
static int secure_rand_below(uint64_t n, uint64_t *out) {
    if (n == 0 || out == NULL) return -1;

    uint64_t x = 0;
    uint64_t limit = (UINT64_MAX / n) * n; // largest multiple of n <= UINT64_MAX

    for (;;) {
        if (get_secure_u64(&x) != 0) return -1;
        if (x < limit) {
            *out = x % n;
            return 0;
        }
        // else reject and retry
    }
}

// Generates a random number strictly greater than min_val and returns it via return value.
// On error, prints a generic error and exits.
uint64_t generate_random_above(uint64_t min_val) {
    if (min_val == UINT64_MAX) {
        fprintf(stderr, "error\n");
        exit(1);
    }
    uint64_t lower = min_val + 1; // safe since min_val != max
    uint64_t remaining = UINT64_MAX - lower; // >= 0
    uint64_t span = (remaining < MAX_SPAN) ? remaining : MAX_SPAN; // in [0, MAX_SPAN]
    uint64_t count = span + 1; // number of values in [lower, lower+span], in [1, MAX_SPAN+1]

    uint64_t r = 0;
    if (secure_rand_below(count, &r) != 0) {
        fprintf(stderr, "error\n");
        exit(1);
    }
    return lower + r;
}

int main(void) {
    uint64_t tests[5] = {
        1000ULL,
        5000ULL,
        12345ULL,
        999999ULL,
        UINT64_MAX - 2ULL
    };

    for (size_t i = 0; i < 5; ++i) {
        uint64_t v = generate_random_above(tests[i]);
        printf("min=%" PRIu64 " -> random=%" PRIu64 "\n", tests[i], v);
        if (!(v > tests[i])) {
            fprintf(stderr, "error\n");
            return 1;
        }
    }
    return 0;
}