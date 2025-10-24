#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #include <stdlib.h> // arc4random_buf
#elif defined(__linux__)
  #include <unistd.h>
  #include <sys/random.h>
  #include <fcntl.h>
#else
  #include <unistd.h>
  #include <fcntl.h>
#endif

static int os_random_bytes(unsigned char* buf, size_t n) {
    if (n == 0) return 0;
#if defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)n, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 0 : -1;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    arc4random_buf(buf, n);
    return 0;
#elif defined(__linux__)
    size_t off = 0;
    while (off < n) {
        ssize_t r = getrandom(buf + off, n - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            if (errno == ENOSYS) break; // fallback to /dev/urandom
            return -1;
        }
        off += (size_t)r;
    }
    if (off < n) {
        int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
        if (fd < 0) return -1;
        while (off < n) {
            ssize_t r = read(fd, buf + off, n - off);
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
    }
    return 0;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, buf + off, n - off);
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
    return 0;
#endif
}

static unsigned char* secure_random_bytes(size_t n) {
    unsigned char* buf = (unsigned char*)malloc(n == 0 ? 1 : n);
    if (!buf && n > 0) return NULL;
    if (n > 0 && os_random_bytes(buf, n) != 0) {
        free(buf);
        return NULL;
    }
    return buf;
}

static uint64_t rand_u64() {
    uint64_t v = 0;
    if (os_random_bytes((unsigned char*)&v, sizeof(v)) != 0) {
        // In case of failure, abort to avoid returning predictable data
        fprintf(stderr, "Fatal: random source unavailable\n");
        exit(1);
    }
    return v;
}

static uint64_t secure_random_uint64(uint64_t min_inclusive, uint64_t max_inclusive) {
    if (min_inclusive > max_inclusive) {
        fprintf(stderr, "Invalid range\n");
        exit(1);
    }
    uint64_t range = max_inclusive - min_inclusive + 1;
    if (range == 0) {
        fprintf(stderr, "Range too large\n");
        exit(1);
    }
    uint64_t limit = UINT64_MAX - (UINT64_MAX % range);
    uint64_t r;
    do {
        r = rand_u64();
    } while (r > limit);
    return min_inclusive + (r % range);
}

static char* secure_hex_token(size_t num_bytes) {
    unsigned char* bytes = secure_random_bytes(num_bytes);
    if (num_bytes > 0 && !bytes) return NULL;
    const char* hex = "0123456789abcdef";
    size_t out_len = num_bytes * 2;
    char* out = (char*)malloc(out_len + 1);
    if (!out) {
        free(bytes);
        return NULL;
    }
    for (size_t i = 0; i < num_bytes; ++i) {
        out[2*i]     = hex[(bytes[i] >> 4) & 0xF];
        out[2*i + 1] = hex[bytes[i] & 0xF];
    }
    out[out_len] = '\0';
    free(bytes);
    return out;
}

int main(void) {
    // 5 test cases
    char* t1 = secure_hex_token(16);
    if (t1) { printf("Hex token (16 bytes): %s\n", t1); free(t1); }

    uint64_t r1 = secure_random_uint64(1, 100);
    printf("Random uint64 [1, 100]: %llu\n", (unsigned long long)r1);

    char* t2 = secure_hex_token(32);
    if (t2) { printf("Hex token (32 bytes): %s\n", t2); free(t2); }

    char* t3 = secure_hex_token(0);
    if (t3) { printf("Hex token (0 bytes): %s\n", t3); free(t3); }

    uint64_t r2 = secure_random_uint64(0, (1ULL << 48));
    printf("Random uint64 [0, 2^48]: %llu\n", (unsigned long long)r2);

    return 0;
}