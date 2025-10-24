#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static uint64_t splitmix64_next(uint64_t *state) {
    uint64_t z = (*state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

int* generateRandomIntList(int count, int minInclusive, int maxInclusive, uint64_t seed, int use_seed) {
    if (count < 0) return NULL;
    if (minInclusive > maxInclusive) return NULL;
    int *arr = (int*)malloc((size_t)count * sizeof(int));
    if (!arr) return NULL;

    uint64_t state;
    if (use_seed) {
        state = seed ? seed : 0x123456789ABCDEFULL;
    } else {
        uint64_t t = (uint64_t)time(NULL);
        uint64_t p = (uint64_t)(uintptr_t)&state;
        state = t ^ (p << 1) ^ 0x9E3779B97F4A7C15ULL;
    }

    uint64_t bound = (uint64_t)((uint64_t)maxInclusive - (uint64_t)minInclusive + 1ULL);
    uint64_t limit = UINT64_MAX - (UINT64_MAX % bound);

    for (int i = 0; i < count; ++i) {
        uint64_t x = splitmix64_next(&state);
        while (x >= limit) {
            x = splitmix64_next(&state);
        }
        arr[i] = (int)((uint64_t)minInclusive + (x % bound));
    }
    return arr;
}

char* generateToken(int length, const char* allowedChars, uint64_t seed, int use_seed) {
    if (length < 0) return NULL;
    const char* defaultChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const char* chars = (allowedChars && allowedChars[0] != '\0') ? allowedChars : defaultChars;
    size_t clen = strlen(chars);

    char *out = (char*)malloc((size_t)length + 1);
    if (!out) return NULL;

    uint64_t state;
    if (use_seed) {
        state = seed ? seed : 0xCAFEBABEDEADBEEFULL;
    } else {
        uint64_t t = (uint64_t)time(NULL);
        uint64_t p = (uint64_t)(uintptr_t)&out;
        state = t ^ (p << 1) ^ 0xD1B54A32D192ED03ULL;
    }

    for (int i = 0; i < length; ++i) {
        uint64_t r = splitmix64_next(&state);
        out[i] = chars[r % clen];
    }
    out[length] = '\0';
    return out;
}

int main(void) {
    printf("C Test Case 1:\n");
    int* a1 = generateRandomIntList(10, 1, 100, 12345ULL, 1);
    if (a1) {
        for (int i = 0; i < 10; ++i) printf("%s%d", i ? "," : "", a1[i]);
        printf("\n");
        free(a1);
    }
    char* t1 = generateToken(16, NULL, 12345ULL, 1);
    if (t1) { printf("%s\n", t1); free(t1); }

    printf("C Test Case 2:\n");
    int* a2 = generateRandomIntList(5, 0, 9, 0ULL, 0);
    if (a2) {
        for (int i = 0; i < 5; ++i) printf("%s%d", i ? "," : "", a2[i]);
        printf("\n");
        free(a2);
    }
    char* t2 = generateToken(8, NULL, 0ULL, 0);
    if (t2) { printf("%s\n", t2); free(t2); }

    printf("C Test Case 3:\n");
    int* a3 = generateRandomIntList(7, -50, 50, 999ULL, 1);
    if (a3) {
        for (int i = 0; i < 7; ++i) printf("%s%d", i ? "," : "", a3[i]);
        printf("\n");
        free(a3);
    }
    char* t3 = generateToken(12, "ABCDEF0123", 42ULL, 1);
    if (t3) { printf("%s\n", t3); free(t3); }

    printf("C Test Case 4:\n");
    int* a4 = generateRandomIntList(3, 1000, 2000, 55555ULL, 1);
    if (a4) {
        for (int i = 0; i < 3; ++i) printf("%s%d", i ? "," : "", a4[i]);
        printf("\n");
        free(a4);
    }
    char* t4 = generateToken(24, "abcdef", 55555ULL, 1);
    if (t4) { printf("%s\n", t4); free(t4); }

    printf("C Test Case 5:\n");
    int* a5 = generateRandomIntList(0, 0, 10, 2021ULL, 1);
    if (a5) {
        for (int i = 0; i < 0; ++i) printf("%s%d", i ? "," : "", a5[i]);
        printf("\n");
        free(a5);
    } else {
        printf("[]\n");
    }
    char* t5 = generateToken(0, NULL, 2021ULL, 1);
    if (t5) { printf("[%s]\n", t5); free(t5); }

    return 0;
}