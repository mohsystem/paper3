#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/syscall.h>
#endif

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    double radius;
    double x_center;
    double y_center;
    int valid;
} Solution;

static int get_secure_bytes(void* buf, size_t len) {
#if defined(__linux__)
    size_t offset = 0;
    while (offset < len) {
        ssize_t n = syscall(SYS_getrandom, (char*)buf + offset, len - offset, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            break; // fallback to /dev/urandom
        }
        offset += (size_t)n;
    }
    if (offset == len) return 0;
#endif
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t offset2 = 0;
    while (offset2 < len) {
        ssize_t n = read(fd, (char*)buf + offset2, len - offset2);
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (n == 0) {
            close(fd);
            return -1;
        }
        offset2 += (size_t)n;
    }
    close(fd);
    return 0;
}

static uint64_t secure_random_u64(void) {
    uint64_t v = 0;
    if (get_secure_bytes(&v, sizeof(v)) != 0) {
        fprintf(stderr, "Secure random generation failed\n");
        exit(1);
    }
    return v;
}

// Uniform double in [0,1)
static double uniform01(void) {
    uint64_t r = secure_random_u64();
    r >>= 11; // keep top 53 bits
    return (double)r * (1.0 / 9007199254740992.0); // 2^53
}

int Solution_init(Solution* s, double radius, double x_center, double y_center) {
    if (s == NULL) return -1;
    if (!(radius > 0.0) || !isfinite(radius)) return -1;
    if (!isfinite(x_center) || !isfinite(y_center)) return -1;
    s->radius = radius;
    s->x_center = x_center;
    s->y_center = y_center;
    s->valid = 1;
    return 0;
}

Point randPoint(const Solution* s) {
    Point p = {0.0, 0.0};
    if (s == NULL || !s->valid) {
        return p;
    }
    double u1 = uniform01();
    double u2 = uniform01();
    double r = sqrt(u1) * s->radius;
    double theta = 2.0 * M_PI * u2;
    p.x = s->x_center + r * cos(theta);
    p.y = s->y_center + r * sin(theta);
    return p;
}

int main(void) {
    // 5 test cases (calls)
    Solution s;
    if (Solution_init(&s, 1.0, 0.0, 0.0) != 0) {
        fprintf(stderr, "Failed to initialize Solution\n");
        return 1;
    }
    for (int i = 0; i < 5; ++i) {
        Point p = randPoint(&s);
        printf("[%.5f, %.5f]\n", p.x, p.y);
    }

    // Additional example tests
    Solution s2;
    if (Solution_init(&s2, 2.0, 3.0, -4.0) != 0) {
        fprintf(stderr, "Failed to initialize Solution 2\n");
        return 1;
    }
    for (int i = 0; i < 5; ++i) {
        Point p = randPoint(&s2);
        printf("[%.5f, %.5f]\n", p.x, p.y);
    }
    return 0;
}