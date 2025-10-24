#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#endif

// Chain-of-Through: 1) Problem + 2) Security
// - Maintain unique active names, generate unpredictable names.
// - In C, use OS randomness: /dev/urandom on POSIX; CryptGenRandom on Windows.
// - Registry implemented with a dynamic array (sufficient for demo/testing scale).

typedef struct {
    char (*names)[6]; // array of 6-char null-terminated strings
    size_t count;
    size_t capacity;
} NameRegistry;

static NameRegistry REGISTRY = { NULL, 0, 0 };

static void registry_init_if_needed() {
    if (REGISTRY.names == NULL) {
        REGISTRY.capacity = 1024;
        REGISTRY.names = (char (*)[6])calloc(REGISTRY.capacity, sizeof(*REGISTRY.names));
        if (!REGISTRY.names) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
}

static int registry_contains(const char name[6]) {
    for (size_t i = 0; i < REGISTRY.count; i++) {
        if (strncmp(REGISTRY.names[i], name, 6) == 0) {
            return 1;
        }
    }
    return 0;
}

static void registry_add(const char name[6]) {
    registry_init_if_needed();
    if (REGISTRY.count == REGISTRY.capacity) {
        size_t newcap = REGISTRY.capacity * 2;
        char (*newnames)[6] = (char (*)[6])realloc(REGISTRY.names, newcap * sizeof(*REGISTRY.names));
        if (!newnames) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        REGISTRY.names = newnames;
        REGISTRY.capacity = newcap;
    }
    memcpy(REGISTRY.names[REGISTRY.count], name, 6);
    REGISTRY.count++;
}

static void registry_remove(const char name[6]) {
    for (size_t i = 0; i < REGISTRY.count; i++) {
        if (strncmp(REGISTRY.names[i], name, 6) == 0) {
            // swap with last and reduce count
            if (i != REGISTRY.count - 1) {
                memcpy(REGISTRY.names[i], REGISTRY.names[REGISTRY.count - 1], 6);
            }
            REGISTRY.count--;
            return;
        }
    }
}

static int is_active(const char name[6]) {
    return registry_contains(name);
}

static int secure_random_bytes(unsigned char* buf, size_t len) {
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        return 0;
    }
    BOOL ok = CryptGenRandom(hProv, (DWORD)len, buf);
    CryptReleaseContext(hProv, 0);
    return ok ? 1 : 0;
#else
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t rd = fread(buf, 1, len, f);
    fclose(f);
    return rd == len ? 1 : 0;
#endif
}

static void random_name(char out[6]) {
    unsigned char bytes[5];
    if (!secure_random_bytes(bytes, sizeof(bytes))) {
        // Fallback (less secure): use rand; seed once
        static int seeded = 0;
        if (!seeded) {
            seeded = 1;
            srand((unsigned)time(NULL));
        }
        for (int i = 0; i < 5; i++) bytes[i] = (unsigned char)(rand() & 0xFF);
    }
    out[0] = 'A' + (bytes[0] % 26);
    out[1] = 'A' + (bytes[1] % 26);
    out[2] = '0' + (bytes[2] % 10);
    out[3] = '0' + (bytes[3] % 10);
    out[4] = '0' + (bytes[4] % 10);
    out[5] = '\0';
}

static void acquire_unique_name(char out[6]) {
    const int MAX_ATTEMPTS = 1000000;
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        char candidate[6];
        random_name(candidate);
        if (!registry_contains(candidate)) {
            memcpy(out, candidate, 6);
            registry_add(out);
            return;
        }
    }
    fprintf(stderr, "Unable to generate a unique robot name after many attempts.\n");
    exit(1);
}

typedef struct {
    char name[6];
    int has_name;
} Robot;

static void robot_init(Robot* r) {
    if (!r) return;
    r->name[0] = '\0';
    r->has_name = 0;
}

static const char* robot_get_name(Robot* r) {
    if (!r) return NULL;
    if (!r->has_name) {
        acquire_unique_name(r->name);
        r->has_name = 1;
    }
    return r->name;
}

static void robot_reset(Robot* r) {
    if (!r) return;
    if (r->has_name) {
        registry_remove(r->name);
        r->name[0] = '\0';
        r->has_name = 0;
    }
}

// Chain-of-Through: 3) Secure coding + 4) Review
// - Uses OS RNG where available; releases names on reset; simple registry for demo scale.

int main(void) {
    // Test 1
    Robot r1;
    robot_init(&r1);
    const char* n1 = robot_get_name(&r1);
    printf("Test1 r1 name: %s active=%s\n", n1, is_active(n1) ? "true" : "false");

    // Test 2
    Robot r2;
    robot_init(&r2);
    const char* n2 = robot_get_name(&r2);
    printf("Test2 r2 name: %s unique_vs_r1=%s\n", n2, strcmp(n1, n2) != 0 ? "true" : "false");

    // Test 3
    const char* n2_again = robot_get_name(&r2);
    printf("Test3 r2 stable: %s == %s -> %s\n", n2, n2_again, strcmp(n2, n2_again) == 0 ? "true" : "false");

    // Test 4
    robot_reset(&r2);
    const char* n2_new = robot_get_name(&r2);
    printf("Test4 r2 reset new name: %s old_different=%s\n", n2_new, strcmp(n2, n2_new) != 0 ? "true" : "false");

    // Test 5
    int count = 1000;
    Robot* robots = (Robot*)calloc((size_t)count, sizeof(Robot));
    if (!robots) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    char** names = (char**)calloc((size_t)count, sizeof(char*));
    if (!names) {
        free(robots);
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    int unique = 1;
    for (int i = 0; i < count; i++) {
        robot_init(&robots[i]);
        const char* nm = robot_get_name(&robots[i]);
        names[i] = strdup(nm);
        if (!names[i]) { unique = 0; break; }
    }
    // check uniqueness (O(n^2) for simplicity)
    for (int i = 0; i < count && unique; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(names[i], names[j]) == 0) {
                unique = 0;
                break;
            }
        }
    }
    printf("Test5 1000 robots unique=%s size=%d\n", unique ? "true" : "false", count);

    for (int i = 0; i < count; i++) free(names[i]);
    free(names);
    free(robots);
    return 0;
}