#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
  #include <windows.h>
  #include <bcrypt.h>
  #pragma comment(lib, "bcrypt")
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #include <stdlib.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/random.h>
  #include <sys/syscall.h>
  #ifndef GRND_NONBLOCK
    #define GRND_NONBLOCK 0x0001
  #endif
#endif

static int csprng_fill(unsigned char* buf, size_t len) {
    if (buf == NULL || len == 0) return 0;
#if defined(_WIN32)
    NTSTATUS st = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return (st == 0) ? 0 : -1;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    arc4random_buf(buf, len);
    return 0;
#else
    size_t off = 0;
    while (off < len) {
        ssize_t r = getrandom(buf + off, len - off, 0);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        off += (size_t)r;
    }
    return 0;
#endif
}

char* random_ascii_letters(size_t length) {
    const size_t MAX_LEN = 1000000u;
    static const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const unsigned letters_len = 52u;
    const unsigned limit = (256u / 52u) * 52u; /* 208 */

    if (length > MAX_LEN) {
        return NULL;
    }

    char* out = (char*)malloc(length + 1);
    if (!out) {
        return NULL;
    }

    size_t produced = 0;
    unsigned char buf[64];
    size_t idx = 0, avail = 0;

    while (produced < length) {
        if (idx >= avail) {
            if (csprng_fill(buf, sizeof(buf)) != 0) {
                free(out);
                return NULL;
            }
            idx = 0;
            avail = sizeof(buf);
        }
        unsigned v = buf[idx++];
        if (v < limit) {
            out[produced++] = letters[v % letters_len];
        }
    }
    out[length] = '\0';
    return out;
}

int main(void) {
    size_t tests[5] = {0, 1, 10, 32, 100};
    for (size_t i = 0; i < 5; ++i) {
        char* s = random_ascii_letters(tests[i]);
        if (s == NULL && tests[i] != 0) {
            fprintf(stderr, "Error: RNG or allocation failure for len=%zu\n", tests[i]);
            continue;
        }
        printf("len=%zu str=%s\n", tests[i], s ? s : "");
        free(s);
    }
    return 0;
}