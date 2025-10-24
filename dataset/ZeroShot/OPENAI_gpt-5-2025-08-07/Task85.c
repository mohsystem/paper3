#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
static int secure_random_bytes(uint8_t* buf, size_t len) {
    return BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
}
#elif defined(__APPLE__)
#include <stdlib.h>
static int secure_random_bytes(uint8_t* buf, size_t len) {
    arc4random_buf(buf, len);
    return 1;
}
#elif defined(__linux__)
#include <sys/random.h>
#include <unistd.h>
#include <fcntl.h>
static int secure_random_bytes(uint8_t* buf, size_t len) {
    ssize_t r = getrandom(buf, len, 0);
    if (r == (ssize_t)len) return 1;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n <= 0) { close(fd); return 0; }
        total += (size_t)n;
    }
    close(fd);
    return 1;
}
#else
#include <unistd.h>
#include <fcntl.h>
static int secure_random_bytes(uint8_t* buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(fd, buf + total, len - total);
        if (n <= 0) { close(fd); return 0; }
        total += (size_t)n;
    }
    close(fd);
    return 1;
}
#endif

char* generate_random_string(size_t length) {
    const char* letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const size_t alphabet = 52;
    char* out = (char*)malloc(length + 1);
    if (!out) return NULL;
    if (length == 0) {
        out[0] = '\0';
        return out;
    }
    uint32_t m = 256 - (256 % alphabet); // 208 to avoid modulo bias
    uint8_t buf[128];
    size_t idx = sizeof(buf);
    size_t pos = 0;
    while (pos < length) {
        if (idx >= sizeof(buf)) {
            if (!secure_random_bytes(buf, sizeof(buf))) {
                free(out);
                return NULL;
            }
            idx = 0;
        }
        uint8_t b = buf[idx++];
        if (b >= m) continue;
        out[pos++] = letters[b % alphabet];
    }
    out[length] = '\0';
    return out;
}

int main(void) {
    size_t tests[5] = {0, 1, 10, 32, 64};
    for (int i = 0; i < 5; ++i) {
        char* s = generate_random_string(tests[i]);
        if (!s) {
            fprintf(stderr, "Error generating random string\n");
            return 1;
        }
        printf("%zu: %s\n", tests[i], s);
        free(s);
    }
    return 0;
}