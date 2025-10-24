#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif defined(__APPLE__)
#include <stdlib.h>
#elif defined(__linux__)
#include <sys/random.h>
#include <errno.h>
#include <unistd.h>
#else
#include <time.h>
#include <random>
#endif

#define MIN_BYTES 16
#define MAX_BYTES 64

static int get_random_bytes(uint8_t* buf, size_t len) {
    if (buf == NULL || len == 0) return 0;
#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, (PUCHAR)buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 1 : 0;
#elif defined(__APPLE__)
    arc4random_buf(buf, len);
    return 1;
#elif defined(__linux__)
    size_t total = 0;
    while (total < len) {
        ssize_t n = getrandom(buf + total, len - total, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return 0;
        }
        total += (size_t)n;
    }
    return 1;
#else
    // Fallback: use /dev/urandom-like source may not be available; as a last resort, use rand_s if available or similar.
    // Here we fallback to stdlib's rand seeded once; not ideal for cryptographic purposes but keeps portability.
    static int seeded = 0;
    if (!seeded) {
        seeded = 1;
        srand((unsigned int)time(NULL));
    }
    for (size_t i = 0; i < len; ++i) {
        buf[i] = (uint8_t)(rand() & 0xFF);
    }
    return 1;
#endif
}

static char* base64url_nopad(const uint8_t* data, size_t n) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    if (data == NULL && n != 0) return NULL;

    size_t full = n / 3;
    size_t rem = n % 3;
    size_t out_len = full * 4 + (rem == 0 ? 0 : (rem + 1));
    char* out = (char*)malloc(out_len + 1);
    if (!out) return NULL;

    size_t oi = 0;
    size_t i = 0;
    for (size_t blk = 0; blk < full; ++blk) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | (uint32_t)data[i + 2];
        out[oi++] = tbl[(v >> 18) & 0x3F];
        out[oi++] = tbl[(v >> 12) & 0x3F];
        out[oi++] = tbl[(v >> 6) & 0x3F];
        out[oi++] = tbl[v & 0x3F];
        i += 3;
    }
    if (rem == 1) {
        uint32_t v = ((uint32_t)data[i] << 16);
        out[oi++] = tbl[(v >> 18) & 0x3F];
        out[oi++] = tbl[(v >> 12) & 0x3F];
    } else if (rem == 2) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8);
        out[oi++] = tbl[(v >> 18) & 0x3F];
        out[oi++] = tbl[(v >> 12) & 0x3F];
        out[oi++] = tbl[(v >> 6) & 0x3F];
    }
    out[oi] = '\0';
    return out;
}

char* generate_session_id(size_t num_bytes) {
    if (num_bytes < MIN_BYTES || num_bytes > MAX_BYTES) {
        return NULL;
    }
    uint8_t* buf = (uint8_t*)malloc(num_bytes);
    if (!buf) return NULL;
    if (!get_random_bytes(buf, num_bytes)) {
        free(buf);
        return NULL;
    }
    char* out = base64url_nopad(buf, num_bytes);
    free(buf);
    return out;
}

int main(void) {
    size_t tests[5] = {16, 24, 32, 48, 64};
    for (int i = 0; i < 5; ++i) {
        char* id = generate_session_id(tests[i]);
        if (id) {
            printf("%s\n", id);
            free(id);
        } else {
            printf("Error\n");
        }
    }
    return 0;
}