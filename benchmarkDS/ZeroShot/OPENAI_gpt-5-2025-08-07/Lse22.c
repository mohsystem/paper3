#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

static int fill_random(void* buf, size_t len) {
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 0 : -1;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, (unsigned char*)buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        off += (size_t)r;
    }
    close(fd);
    return 0;
#endif
}

static int random_u64(uint64_t* out) {
    if (!out) return -1;
    uint64_t v = 0;
    if (fill_random(&v, sizeof(v)) != 0) return -1;
    *out = v;
    return 0;
}

int generate_random_greater_than(uint64_t minExclusive, uint64_t* out) {
    if (!out) return -1;
    if (minExclusive == UINT64_MAX) return -2;
    uint64_t v = 0;
    do {
        if (random_u64(&v) != 0) return -3;
    } while (v <= minExclusive);
    *out = v;
    return 0;
}

int main(void) {
    for (int i = 0; i < 5; ++i) {
        uint64_t v;
        if (generate_random_greater_than(1000, &v) != 0) {
            return 1;
        }
        printf("%" PRIu64 "\n", v);
    }
    return 0;
}