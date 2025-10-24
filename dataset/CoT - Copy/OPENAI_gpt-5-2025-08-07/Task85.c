#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#ifdef _MSC_VER
#pragma comment(lib, "bcrypt.lib")
#endif
#else
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __linux__
#include <sys/random.h>
#endif
#endif

static int get_random_bytes(unsigned char* buf, size_t len) {
#ifdef _WIN32
    return BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#else
#ifdef __linux__
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(buf + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            break;
        }
        off += (size_t)r;
    }
    if (off == len) return 1;
#endif
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return 0;
    size_t off = 0;
    while (off < len) {
        ssize_t r = read(fd, buf + off, len - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return 0;
        }
        if (r == 0) {
            close(fd);
            return 0;
        }
        off += (size_t)r;
    }
    close(fd);
    return 1;
#endif
}

char* generate_random_string(size_t length) {
    static const char* letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const size_t alphabet = 52;
    unsigned int limit = 256 - (256 % alphabet);

    char* out = (char*)malloc(length + 1);
    if (!out && length != 0) return NULL;

    size_t idx = 0;
    while (idx < length) {
        unsigned char buffer[64];
        if (!get_random_bytes(buffer, sizeof(buffer))) {
            free(out);
            return NULL;
        }
        for (size_t i = 0; i < sizeof(buffer) && idx < length; i++) {
            unsigned char b = buffer[i];
            if (b < limit) {
                out[idx++] = letters[b % alphabet];
            }
        }
    }
    out[length] = '\0';
    return out;
}

int main(void) {
    size_t tests[5] = {0, 1, 16, 32, 100};
    for (int i = 0; i < 5; i++) {
        char* s = generate_random_string(tests[i]);
        if (!s) {
            printf("%zu: [error]\n", tests[i]);
        } else {
            printf("%zu: %s\n", tests[i], s);
            free(s);
        }
    }
    return 0;
}