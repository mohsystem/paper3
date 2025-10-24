#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

typedef struct {
    double x;
    double y;
} Point;

typedef struct {
    double radius;
    double x_center;
    double y_center;
    uint64_t rng_state; /* Non-zero state for xorshift64* */
} Solution;

/* Secure-ish zero to avoid compiler optimizing out */
static void portable_explicit_bzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
}

/* Fallback entropy if OS CSPRNG is unavailable */
static uint64_t fallback_entropy(void) {
    uint64_t s = 0x9E3779B97F4A7C15ULL;
    /* Combine multiple system values */
    {
        time_t t = time(NULL);
        clock_t c = clock();
        uint64_t mix = ((uint64_t)(t)) ^ (((uint64_t)c) << 32);
        uintptr_t addr = (uintptr_t)&mix;
        mix ^= ((uint64_t)addr << 13) | ((uint64_t)addr >> 7);
        s ^= mix;
    }
#ifdef _WIN32
    LARGE_INTEGER li;
    if (QueryPerformanceCounter(&li)) {
        s ^= (uint64_t)li.QuadPart;
    }
#else
    struct timespec ts;
#if defined(TIME_UTC)
    if (timespec_get(&ts, TIME_UTC) == TIME_UTC) {
        s ^= ((uint64_t)ts.tv_sec << 32) ^ (uint64_t)ts.tv_nsec;
    } else {
        /* best effort */
        ts.tv_sec = (time_t)time(NULL);
        ts.tv_nsec = 0u;
        s ^= ((uint64_t)ts.tv_sec << 32);
    }
#else
    ts.tv_sec = (time_t)time(NULL);
    ts.tv_nsec = 0u;
    s ^= ((uint64_t)ts.tv_sec << 32);
#endif
#endif
    /* A couple of rounds of SplitMix64 to decorrelate */
    uint64_t z = s + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    s = z ^ (z >> 31);

    z = s + 0x9E3779B97F4A7C15ULL;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    s ^= z ^ (z >> 31);
    if (s == 0) s = 0xD1B54A32D192ED03ULL; /* ensure non-zero */
    return s;
}

/* Try to acquire 64-bit seed from OS CSPRNG; fall back if necessary */
static uint64_t get_seed64(void) {
    uint64_t seed = 0;
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        if (CryptGenRandom(hProv, (DWORD)sizeof(seed), (BYTE *)&seed)) {
            CryptReleaseContext(hProv, 0);
            if (seed != 0) return seed;
        } else {
            CryptReleaseContext(hProv, 0);
        }
    }
#else
    FILE *f = fopen("/dev/urandom", "rb");
    if (f != NULL) {
        size_t r = fread(&seed, 1, sizeof(seed), f);
        fclose(f);
        if (r == sizeof(seed) && seed != 0) {
            return seed;
        }
    }
#endif
    return fallback_entropy();
}

/* xorshift64* RNG: state must be non-zero */
static uint64_t xs64star_next(uint64_t *state) {
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

/* Produce a uniform double in [0,1) using 53 random bits */
static double rng_uniform01(uint64_t *state) {
    uint64_t r = xs64star_next(state);
    /* Take top 53 bits for double fraction */
    const double inv = 1.0 / 9007199254740992.0; /* 2^53 */
    return ((r >> 11) & 0x1FFFFFFFFFFFFFULL) * inv;
}

/* Constructor-like function: returns a fully initialized Solution */
static Solution Solution_create(double radius, double x_center, double y_center) {
    Solution s;
    /* Input validation: ensure finite values and non-negative radius */
    if (!(isfinite(radius)) || radius < 0.0) {
        radius = 0.0;
    }
    if (!isfinite(x_center)) x_center = 0.0;
    if (!isfinite(y_center)) y_center = 0.0;

    s.radius = radius;
    s.x_center = x_center;
    s.y_center = y_center;

    uint64_t seed = get_seed64();
    if (seed == 0) seed = 0xA5A5A5A5A5A5A5A5ULL; /* last resort non-zero */
    s.rng_state = seed;
    return s;
}

/* Generate a random point uniformly within or on the circle */
static Point Solution_randPoint(Solution *sol) {
    Point p;
    if (sol == NULL) {
        p.x = 0.0;
        p.y = 0.0;
        return p;
    }
    if (sol->radius <= 0.0) {
        p.x = sol->x_center;
        p.y = sol->y_center;
        return p;
    }
    double u = rng_uniform01(&sol->rng_state);
    double v = rng_uniform01(&sol->rng_state);
    double r = sqrt(u) * sol->radius;
    double theta = v * (2.0 * M_PI);
    double ct = cos(theta);
    double st = sin(theta);
    p.x = sol->x_center + r * ct;
    p.y = sol->y_center + r * st;
    return p;
}

/* Simple test harness with 5 test cases */
int main(void) {
    /* Enable consistent output formatting */
    printf("Test 1: radius=1.0, center=(0.0,0.0)\n");
    Solution s1 = Solution_create(1.0, 0.0, 0.0);
    for (int i = 0; i < 3; ++i) {
        Point p = Solution_randPoint(&s1);
        printf("  randPoint -> [%.5f, %.5f]\n", p.x, p.y);
    }

    printf("Test 2: radius=2.0, center=(5.0,-3.5)\n");
    Solution s2 = Solution_create(2.0, 5.0, -3.5);
    for (int i = 0; i < 3; ++i) {
        Point p = Solution_randPoint(&s2);
        printf("  randPoint -> [%.5f, %.5f]\n", p.x, p.y);
    }

    printf("Test 3: radius=0.1, center=(1e7,-1e7)\n");
    Solution s3 = Solution_create(0.1, 1e7, -1e7);
    for (int i = 0; i < 3; ++i) {
        Point p = Solution_randPoint(&s3);
        printf("  randPoint -> [%.5f, %.5f]\n", p.x, p.y);
    }

    printf("Test 4: radius=1e8, center=(0.0,0.0)\n");
    Solution s4 = Solution_create(1e8, 0.0, 0.0);
    for (int i = 0; i < 3; ++i) {
        Point p = Solution_randPoint(&s4);
        printf("  randPoint -> [%.5f, %.5f]\n", p.x, p.y);
    }

    printf("Test 5: invalid radius (negative), center=(2.0,3.0) -> treated as 0 radius\n");
    Solution s5 = Solution_create(-5.0, 2.0, 3.0);
    for (int i = 0; i < 3; ++i) {
        Point p = Solution_randPoint(&s5);
        printf("  randPoint -> [%.5f, %.5f]\n", p.x, p.y);
    }

    /* Zero sensitive data (rng state) before exit */
    portable_explicit_bzero(&s1, sizeof(s1));
    portable_explicit_bzero(&s2, sizeof(s2));
    portable_explicit_bzero(&s3, sizeof(s3));
    portable_explicit_bzero(&s4, sizeof(s4));
    portable_explicit_bzero(&s5, sizeof(s5));
    return 0;
}