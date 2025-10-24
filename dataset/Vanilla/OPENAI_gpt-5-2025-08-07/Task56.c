#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

#if !defined(_WIN32) && !defined(_WIN64)
#include <fcntl.h>
#include <unistd.h>
#endif

static const char* ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static int secure_random_bytes(unsigned char* buf, size_t len) {
#if defined(_WIN32) || defined(_WIN64)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 0 : -1;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) return -1;
    size_t total = 0;
    while (total < len) {
        ssize_t r = read(fd, buf + total, len - total);
        if (r <= 0) { close(fd); return -1; }
        total += (size_t)r;
    }
    close(fd);
    return 0;
#endif
}

char* generateToken(const char* userId, int length) {
    int n = length < 16 ? 16 : length;
    char* token = (char*)malloc((size_t)n + 1);
    if (!token) return NULL;

    unsigned char* rnd = (unsigned char*)malloc((size_t)n);
    if (!rnd) { free(token); return NULL; }

    if (secure_random_bytes(rnd, (size_t)n) != 0) {
        // Fallback to PRNG if secure RNG unavailable
        unsigned long seed = (unsigned long)time(NULL);
        seed ^= (unsigned long)(uintptr_t)&n;
        if (userId) {
            for (const char* p = userId; *p; ++p) {
                seed = seed * 1664525u + (unsigned char)(*p) + 1013904223u;
            }
        }
        srand((unsigned int)seed);
        for (int i = 0; i < n; ++i) {
            rnd[i] = (unsigned char)(rand() & 0xFF);
        }
    }

    size_t alen = strlen(ALPHABET);
    for (int i = 0; i < n; ++i) {
        token[i] = ALPHABET[rnd[i] % alen];
    }
    token[n] = '\0';

    free(rnd);
    return token;
}

int main(void) {
    char* t1 = generateToken("user1", 32);
    char* t2 = generateToken("user2", 24);
    char* t3 = generateToken("user3", 48);
    char* t4 = generateToken("user4", 64);
    char* t5 = generateToken("user5", 32);

    if (t1) { printf("user1: %s\n", t1); free(t1); }
    if (t2) { printf("user2: %s\n", t2); free(t2); }
    if (t3) { printf("user3: %s\n", t3); free(t3); }
    if (t4) { printf("user4: %s\n", t4); free(t4); }
    if (t5) { printf("user5: %s\n", t5); free(t5); }

    return 0;
}