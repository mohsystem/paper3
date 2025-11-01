#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static int read_fully(int fd, void* buf, size_t size) {
    size_t off = 0;
    while (off < size) {
        ssize_t n = read(fd, (unsigned char*)buf + off, size - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (n == 0) return -1; // unexpected EOF
        off += (size_t)n;
    }
    return 0;
}

static uint32_t secure_random_u32(int urandom_fd) {
    uint32_t v = 0;
    if (read_fully(urandom_fd, &v, sizeof(v)) != 0) {
        // In case of failure, fallback to a minimal safe behavior
        // Return 0 to avoid undefined state; callers map to range with rejection.
        return 0;
    }
    return v;
}

static uint32_t uniform_u32_inclusive(int urandom_fd, uint32_t max_inclusive) {
    uint64_t range = (uint64_t)max_inclusive + 1ULL; // size of range
    uint64_t limit = (UINT64_C(0x100000000) / range) * range - 1ULL; // 2^32 aligned limit
    for (;;) {
        uint32_t r = secure_random_u32(urandom_fd);
        if ((uint64_t)r <= limit) {
            return (uint32_t)((uint64_t)r % range);
        }
    }
}

int getMonthlySales(int urandom_fd) {
    return (int)uniform_u32_inclusive(urandom_fd, 100000);
}

void computeQuarterlySales(int urandom_fd, long long out_quarters[4]) {
    out_quarters[0] = out_quarters[1] = out_quarters[2] = out_quarters[3] = 0;
    for (int month = 0; month < 12; ++month) {
        int sale = getMonthlySales(urandom_fd);
        out_quarters[month / 3] += sale;
    }
}

int main(void) {
    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        fprintf(stderr, "Failed to open /dev/urandom: %s\n", strerror(errno));
        return 1;
    }

    for (int i = 1; i <= 5; ++i) {
        long long q[4];
        computeQuarterlySales(urandom_fd, q);
        printf("Test case %d: Q1: %lld, Q2: %lld, Q3: %lld, Q4: %lld\n",
               i, q[0], q[1], q[2], q[3]);
    }

    close(urandom_fd);
    return 0;
}