#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#if defined(__linux__)
#include <sys/random.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <stdlib.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

// Registry of names (simple dynamic array with linear search for simplicity)
typedef struct {
    char** names;
    size_t count;
    size_t capacity;
} Registry;

typedef struct {
    char* name; // NULL until first requested
    Registry* registry; // reference to shared registry
} Robot;

static const size_t MAX_NAMES = 26u * 26u * 1000u;

// Secure randomness
static int secure_random_bytes(unsigned char* buf, size_t len) {
#if defined(__linux__)
    size_t total = 0;
    while (total < len) {
        ssize_t r = getrandom(buf + total, len - total, 0);
        if (r <= 0) return -1;
        total += (size_t)r;
    }
    return 0;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    arc4random_buf(buf, len);
    return 0;
#elif defined(_WIN32)
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status == 0 ? 0 : -1;
#else
    return -1;
#endif
}

static int secure_random_u32(uint32_t* out) {
    unsigned char b[4];
    if (secure_random_bytes(b, sizeof(b)) != 0) return -1;
    *out = ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
    return 0;
}

static int uniform_below(uint32_t upper_exclusive, uint32_t* out) {
    if (upper_exclusive == 0) { *out = 0; return 0; }
    uint32_t limit = 0xFFFFFFFFu - (0xFFFFFFFFu % upper_exclusive);
    while (1) {
        uint32_t r = 0;
        if (secure_random_u32(&r) != 0) return -1;
        if (r <= limit) {
            *out = r % upper_exclusive;
            return 0;
        }
    }
}

// Registry functions
static void registry_init(Registry* reg) {
    reg->names = NULL;
    reg->count = 0;
    reg->capacity = 0;
}

static void registry_free(Registry* reg) {
    if (!reg) return;
    for (size_t i = 0; i < reg->count; i++) {
        free(reg->names[i]);
    }
    free(reg->names);
    reg->names = NULL;
    reg->count = 0;
    reg->capacity = 0;
}

static int registry_contains(const Registry* reg, const char* name) {
    if (!reg || !name) return 0;
    for (size_t i = 0; i < reg->count; i++) {
        if (reg->names[i] && strcmp(reg->names[i], name) == 0) return 1;
    }
    return 0;
}

static int registry_add(Registry* reg, const char* name) {
    if (!reg || !name) return 0;
    if (registry_contains(reg, name)) return 0;
    if (reg->count == reg->capacity) {
        size_t newcap = reg->capacity == 0 ? 16 : reg->capacity * 2;
        char** nn = (char**)realloc(reg->names, newcap * sizeof(char*));
        if (!nn) return 0;
        reg->names = nn;
        reg->capacity = newcap;
    }
    size_t len = strlen(name);
    char* dup = (char*)malloc(len + 1);
    if (!dup) return 0;
    memcpy(dup, name, len + 1);
    reg->names[reg->count++] = dup;
    return 1;
}

static int registry_remove(Registry* reg, const char* name) {
    if (!reg || !name) return 0;
    for (size_t i = 0; i < reg->count; i++) {
        if (reg->names[i] && strcmp(reg->names[i], name) == 0) {
            free(reg->names[i]);
            reg->names[i] = reg->names[reg->count - 1];
            reg->count--;
            return 1;
        }
    }
    return 0;
}

// Name format validation
static int validate_name_format(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len != 5) return 0;
    for (int i = 0; i < 2; i++) {
        if (name[i] < 'A' || name[i] > 'Z') return 0;
    }
    for (int i = 2; i < 5; i++) {
        if (name[i] < '0' || name[i] > '9') return 0;
    }
    return 1;
}

// Generate candidate
static int generate_candidate_name(char out[6]) {
    uint32_t v;
    if (uniform_below(26, &v) != 0) return 0;
    out[0] = (char)('A' + (int)v);
    if (uniform_below(26, &v) != 0) return 0;
    out[1] = (char)('A' + (int)v);
    for (int i = 2; i < 5; i++) {
        if (uniform_below(10, &v) != 0) return 0;
        out[i] = (char)('0' + (int)v);
    }
    out[5] = '\0';
    return 1;
}

static int acquire_unique_name(Registry* reg, char out[6]) {
    if (!reg || !out) return 0;
    if (reg->count >= MAX_NAMES) return 0;
    for (size_t attempts = 0; attempts < MAX_NAMES * 2u; attempts++) {
        char cand[6];
        if (!generate_candidate_name(cand)) continue;
        if (!validate_name_format(cand)) continue;
        if (!registry_contains(reg, cand)) {
            if (registry_add(reg, cand)) {
                memcpy(out, cand, 6);
                return 1;
            }
        }
    }
    return 0;
}

// Robot API
static void robot_init(Robot* r, Registry* reg) {
    if (!r) return;
    r->name = NULL;
    r->registry = reg;
}

static const char* robot_get_name(Robot* r) {
    if (!r || !r->registry) return NULL;
    if (r->name == NULL) {
        char buf[6];
        if (!acquire_unique_name(r->registry, buf)) return NULL;
        r->name = (char*)malloc(6);
        if (!r->name) {
            registry_remove(r->registry, buf);
            return NULL;
        }
        memcpy(r->name, buf, 6);
    }
    return r->name;
}

static int robot_reset(Robot* r) {
    if (!r || !r->registry) return 0;
    if (r->name == NULL) return 0;
    registry_remove(r->registry, r->name);
    free(r->name);
    r->name = NULL;
    return 1;
}

static void robot_free(Robot* r) {
    if (!r) return;
    if (r->name) {
        // Optionally keep name in registry until reset, but we free and remove to avoid leaks
        registry_remove(r->registry, r->name);
        free(r->name);
        r->name = NULL;
    }
    r->registry = NULL;
}

// Test helpers
static int is_unique(char** names, size_t n) {
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            if (strcmp(names[i], names[j]) == 0) return 0;
        }
    }
    return 1;
}

// Tests
static int test1_single_robot_name_pattern(Registry* reg) {
    Robot r; robot_init(&r, reg);
    const char* n = robot_get_name(&r);
    int ok = (n != NULL) && validate_name_format(n);
    robot_free(&r);
    return ok;
}

static int test2_multiple_robots_unique(Registry* reg) {
    Robot robots[10];
    for (int i = 0; i < 10; i++) robot_init(&robots[i], reg);
    char* names[10];
    int ok = 1;
    for (int i = 0; i < 10; i++) {
        const char* n = robot_get_name(&robots[i]);
        if (!n || !validate_name_format(n)) { ok = 0; }
        names[i] = (char*)n;
    }
    if (ok) ok = is_unique(names, 10);
    for (int i = 0; i < 10; i++) robot_free(&robots[i]);
    return ok;
}

static int test3_reset_changes_and_unique(Registry* reg) {
    Robot r1, r2; robot_init(&r1, reg); robot_init(&r2, reg);
    const char* n1 = robot_get_name(&r1);
    const char* n2 = robot_get_name(&r2);
    if (!n1 || !n2 || strcmp(n1, n2) == 0) { robot_free(&r1); robot_free(&r2); return 0; }
    robot_reset(&r1);
    const char* n1b = robot_get_name(&r1);
    int ok = n1b && strcmp(n1, n1b) != 0 && strcmp(n1b, r2.name) != 0;
    robot_free(&r1); robot_free(&r2);
    return ok;
}

static int test4_repeated_resets_yield_different_names(Registry* reg) {
    Robot r; robot_init(&r, reg);
    char* seen[6];
    int count = 0;
    int ok = 1;
    for (int i = 0; i < 6; i++) {
        const char* n = robot_get_name(&r);
        if (!n) { ok = 0; break; }
        for (int j = 0; j < count; j++) {
            if (strcmp(seen[j], n) == 0) { ok = 0; break; }
        }
        if (!ok) break;
        seen[count++] = (char*)n;
        robot_reset(&r);
    }
    robot_free(&r);
    return ok;
}

static int test5_stress_uniqueness(Registry* reg) {
    const int N = 300; // use 300 for reasonable runtime/memory
    Robot* robots = (Robot*)malloc(sizeof(Robot) * N);
    if (!robots) return 0;
    for (int i = 0; i < N; i++) robot_init(&robots[i], reg);
    char** names = (char**)malloc(sizeof(char*) * N);
    if (!names) { free(robots); return 0; }
    int ok = 1;
    for (int i = 0; i < N; i++) {
        const char* n = robot_get_name(&robots[i]);
        if (!n || !validate_name_format(n)) { ok = 0; break; }
        names[i] = (char*)n;
    }
    if (ok) ok = is_unique(names, (size_t)N);
    for (int i = 0; i < N; i++) robot_free(&robots[i]);
    free(names);
    free(robots);
    return ok;
}

int main(void) {
    Registry reg;
    registry_init(&reg);

    printf("Test1 pattern: %s\n", test1_single_robot_name_pattern(&reg) ? "true" : "false");
    printf("Test2 uniqueness among 10: %s\n", test2_multiple_robots_unique(&reg) ? "true" : "false");
    printf("Test3 reset changes and unique: %s\n", test3_reset_changes_and_unique(&reg) ? "true" : "false");
    printf("Test4 repeated resets yield different names: %s\n", test4_repeated_resets_yield_different_names(&reg) ? "true" : "false");
    printf("Test5 stress uniqueness among 300: %s\n", test5_stress_uniqueness(&reg) ? "true" : "false");

    registry_free(&reg);
    return 0;
}