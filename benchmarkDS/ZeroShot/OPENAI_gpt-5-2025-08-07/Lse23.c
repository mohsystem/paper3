#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static int secure_random_u32(uint32_t *out) {
    if (out == NULL) return -1;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    uint8_t buf[4];
    size_t off = 0;
    while (off < sizeof(buf)) {
        ssize_t r = read(fd, buf + off, sizeof(buf) - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (r == 0) {
            close(fd);
            return -1;
        }
        off += (size_t)r;
    }
    close(fd);
    uint32_t val = ((uint32_t)buf[0]) |
                   ((uint32_t)buf[1] << 8) |
                   ((uint32_t)buf[2] << 16) |
                   ((uint32_t)buf[3] << 24);
    *out = val;
    return 0;
}

static int get_random_in_range(int minVal, int maxVal, int *out) {
    if (out == NULL) return -1;
    if (minVal > maxVal) return -1;
    unsigned long long widthULL = (unsigned long long)( (long long)maxVal - (long long)minVal + 1LL );
    if (widthULL == 0ULL || widthULL > (unsigned long long)UINT32_MAX + 1ULL) return -1;
    uint32_t width = (uint32_t)widthULL;
    uint32_t limit = UINT32_MAX - (UINT32_MAX % width);
    uint32_t r;
    do {
        if (secure_random_u32(&r) != 0) return -1;
    } while (r > limit);
    int val = minVal + (int)(r % width);
    *out = val;
    return 0;
}

int generate_three_and_sum(int minExclusive, int maxInclusive, int results[4]) {
    if (results == NULL) return -1;
    long baseL = (long)minExclusive + 1L;
    if (baseL < 1001L) baseL = 1001L;
    if ((long)maxInclusive < baseL || maxInclusive > INT_MAX) return -1;
    int base = (int)baseL;

    int a, b, c;
    if (get_random_in_range(base, maxInclusive, &a) != 0) return -1;
    if (get_random_in_range(base, maxInclusive, &b) != 0) return -1;
    if (get_random_in_range(base, maxInclusive, &c) != 0) return -1;

    long long sum = (long long)a + (long long)b + (long long)c;
    if (sum > INT_MAX || sum < INT_MIN) return -1;

    results[0] = a;
    results[1] = b;
    results[2] = c;
    results[3] = (int)sum;
    return 0;
}

int main(void) {
    for (int i = 0; i < 5; i++) {
        int res[4];
        if (generate_three_and_sum(1000, 1000000, res) != 0) {
            return 1;
        }
        printf("a : %d b : %d c : %d sum: %d sum2: %d\n", res[0], res[1], res[2], res[3], res[3]);
    }
    return 0;
}