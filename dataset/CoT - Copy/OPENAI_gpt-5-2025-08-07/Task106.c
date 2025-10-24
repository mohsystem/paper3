#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

// Step 1: Problem understanding - RSA 64-bit demo (toy)

typedef uint64_t u64;
typedef __uint128_t u128;

static u64 modmul(u64 a, u64 b, u64 m) {
    return (u128)a * b % m;
}

static u64 modexp(u64 a, u64 e, u64 m) {
    u64 res = 1 % m;
    a %= m;
    while (e) {
        if (e & 1) res = modmul(res, a, m);
        a = modmul(a, a, m);
        e >>= 1;
    }
    return res;
}

static u64 gcd_u64(u64 a, u64 b) {
    while (b) {
        u64 t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static long long egcd(long long a, long long b, long long *x, long long *y) {
    if (b == 0) { *x = 1; *y = 0; return a; }
    long long x1, y1;
    long long g = egcd(b, a % b, &x1, &y1);
    *x = y1;
    *y = x1 - (a / b) * y1;
    return g;
}

static bool modinv(u64 a, u64 m, u64 *inv) {
    long long x, y;
    long long g = egcd((long long)a, (long long)m, &x, &y);
    if (g != 1) return false;
    long long r = x % (long long)m;
    if (r < 0) r += (long long)m;
    *inv = (u64)r;
    return true;
}

static bool isPrime(u64 n) {
    if (n < 2) return false;
    const u64 small[] = {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL};
    for (size_t i = 0; i < sizeof(small)/sizeof(small[0]); ++i) {
        if (n % small[i] == 0) return n == small[i];
    }
    u64 d = n - 1, s = 0;
    while ((d & 1ULL) == 0) { d >>= 1; ++s; }
    const u64 bases[] = {2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL};
    for (size_t i = 0; i < sizeof(bases)/sizeof(bases[0]); ++i) {
        u64 a = bases[i] % n;
        if (a == 0) continue;
        u64 x = modexp(a, d, n);
        if (x == 1 || x == n - 1) continue;
        bool cont = false;
        for (u64 r = 1; r < s; ++r) {
            x = modmul(x, x, n);
            if (x == n - 1) { cont = true; break; }
        }
        if (!cont) return false;
    }
    return true;
}

// Step 2: Security - secure random from /dev/urandom
static u64 random_u64_secure() {
    u64 v = 0;
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t r = read(fd, &v, sizeof(v));
        close(fd);
        if (r == sizeof(v)) return v;
    }
    // Fallback (not ideal)
    v = (u64)rand();
    v = (v << 32) ^ (u64)rand();
    return v;
}

static u64 random_bits(int bits) {
    u64 x = 0;
    int produced = 0;
    while (produced < bits) {
        x <<= 32;
        x ^= (u64)random_u64_secure();
        produced += 32;
    }
    if (bits < 64) {
        x &= ((1ULL << bits) - 1ULL);
    }
    x |= (1ULL << (bits - 1)); // set MSB
    x |= 1ULL; // odd
    return x;
}

static u64 gen_prime_32() {
    for (;;) {
        u64 cand = random_bits(32);
        if (isPrime(cand)) return cand;
    }
}

typedef struct {
    u64 n, e, d;
} RSAKey;

static RSAKey generateKeys() {
    const u64 e = 65537ULL;
    for (;;) {
        u64 p = gen_prime_32();
        u64 q = gen_prime_32();
        if (p == q) continue;
        u64 n = p * q;
        u64 phi = (p - 1) * (q - 1);
        if (gcd_u64(e, phi) != 1) continue;
        u64 d;
        if (!modinv(e, phi, &d)) continue;
        RSAKey k = { n, e, d };
        return k;
    }
}

static u64 encrypt(u64 m, u64 e, u64 n) {
    if (m >= n) {
        fprintf(stderr, "Message out of range\n");
        exit(1);
    }
    return modexp(m, e, n);
}

static u64 decrypt(u64 c, u64 d, u64 n) {
    if (c >= n) {
        fprintf(stderr, "Ciphertext out of range\n");
        exit(1);
    }
    return modexp(c, d, n);
}

static u64 string_to_u64(const char* s) {
    u64 v = 0;
    size_t len = strlen(s);
    for (size_t i = 0; i < len; ++i) {
        unsigned char b = (unsigned char)s[i];
        v = (v << 8) | (u64)b;
    }
    return v;
}

static char* u64_to_string(u64 x) {
    if (x == 0) {
        char* out = (char*)malloc(1);
        if (!out) exit(1);
        out[0] = '\0';
        return out;
    }
    unsigned char tmp[8];
    int idx = 0;
    while (x > 0 && idx < 8) {
        tmp[idx++] = (unsigned char)(x & 0xFF);
        x >>= 8;
    }
    char* out = (char*)malloc((size_t)idx + 1);
    if (!out) exit(1);
    for (int i = 0; i < idx; ++i) {
        out[i] = (char)tmp[idx - 1 - i];
    }
    out[idx] = '\0';
    return out;
}

// Step 4+5: Review and tests
int main(void) {
    RSAKey key = generateKeys();

    const char* tests[5] = { "A", "Hi", "Cat", "Test", "Hello" };

    for (int i = 0; i < 5; ++i) {
        const char* msg = tests[i];
        u64 m = string_to_u64(msg);
        if (m >= key.n) {
            printf("Test %d: message too large for modulus\n", i + 1);
            continue;
        }
        u64 c = encrypt(m, key.e, key.n);
        u64 p = decrypt(c, key.d, key.n);
        char* rec = u64_to_string(p);
        printf("Test %d:\n", i + 1);
        printf("  Message: %s\n", msg);
        printf("  Cipher (hex): 0x%016llx\n", (unsigned long long)c);
        printf("  Decrypted: %s\n", rec);
        printf("  OK: %s\n", strcmp(msg, rec) == 0 ? "true" : "false");
        free(rec);
    }
    return 0;
}