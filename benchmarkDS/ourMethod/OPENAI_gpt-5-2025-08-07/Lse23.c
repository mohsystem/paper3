#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEFAULT_MIN 1001
#define DEFAULT_MAX 1000000

static int urandom_fd = -1;

static int init_urandom(void) {
    if (urandom_fd != -1) {
        return 0;
    }
    int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return -1;
    }
    urandom_fd = fd;
    return 0;
}

static int get_random_bytes(void *buf, size_t len) {
    if (init_urandom() != 0) {
        return -1;
    }
    unsigned char *p = (unsigned char *)buf;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(urandom_fd, p + total, len - total);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        total += (size_t)n;
    }
    return 0;
}

static int secure_random_u32(uint32_t *out) {
    if (out == NULL) return -1;
    return get_random_bytes(out, sizeof(uint32_t));
}

static int secure_random_in_range(int low, int high, int *out) {
    if (out == NULL) return -1;
    if (low > high) return -1;
    uint32_t range = (uint32_t)((uint64_t)(high - low) + 1ULL);
    if (range == 0U) return -1; // overflow or invalid
    uint32_t r = 0U;
    uint32_t limit = UINT32_MAX - (UINT32_MAX % range);
    do {
        if (secure_random_u32(&r) != 0) return -1;
    } while (r > limit);
    *out = low + (int)(r % range);
    return 0;
}

// Returns a newly allocated string containing the formatted output.
// Caller must free the returned string with free().
char *generate_output(int min_value) {
    int low = (min_value > DEFAULT_MIN) ? min_value : DEFAULT_MIN;
    int high = DEFAULT_MAX;
    if (low > high) {
        low = DEFAULT_MIN; // fail closed to safe defaults
    }

    int a = 0, b = 0, c = 0;
    if (secure_random_in_range(low, high, &a) != 0) return NULL;
    if (secure_random_in_range(low, high, &b) != 0) return NULL;
    if (secure_random_in_range(low, high, &c) != 0) return NULL;

    long long sum = (long long)a + (long long)b + (long long)c;
    long long sum2 = (long long)a + (long long)b + (long long)c;

    // Allocate buffer for formatted string
    // Worst case lengths are well within this size.
    size_t bufsize = 256U;
    char *buf = (char *)malloc(bufsize);
    if (!buf) return NULL;

    int n = snprintf(buf, bufsize, "a : %d b : %d c : %d sum: %lld sum2: %lld",
                     a, b, c, sum, sum2);
    if (n < 0 || (size_t)n >= bufsize) {
        free(buf);
        return NULL;
    }
    return buf;
}

int main(void) {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        char *s = generate_output(1001);
        if (s != NULL) {
            puts(s);
            free(s);
        }
    }
    if (urandom_fd != -1) {
        close(urandom_fd);
        urandom_fd = -1;
    }
    return 0;
}