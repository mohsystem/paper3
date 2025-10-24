#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    int has_name;
    char name[6]; // 2 letters + 3 digits + '\0'
} Robot;

static char **allocated_names = NULL;
static size_t allocated_count = 0;
static size_t allocated_capacity = 0;

static int secure_random_bytes(uint8_t *buf, size_t len) {
    // Try /dev/urandom
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        size_t off = 0;
        while (off < len) {
            ssize_t r = read(fd, buf + off, len - off);
            if (r <= 0) break;
            off += (size_t)r;
        }
        close(fd);
        if (off == len) return 1;
    }
    // Fallback to rand (not ideal, but ensures functionality if urandom unavailable)
    static int seeded = 0;
    if (!seeded) {
        unsigned long seed = (unsigned long)time(NULL) ^ (unsigned long)(uintptr_t)&seeded ^ (unsigned long)getpid();
        srand((unsigned int)seed);
        seeded = 1;
    }
    for (size_t i = 0; i < len; i++) {
        buf[i] = (uint8_t)(rand() & 0xFF);
    }
    return 1;
}

static uint8_t get_uniform_u8(uint8_t upper_exclusive) {
    if (upper_exclusive == 0) return 0;
    uint8_t x;
    uint8_t limit = (uint8_t)(256 / upper_exclusive * upper_exclusive); // largest multiple < 256
    do {
        secure_random_bytes(&x, 1);
    } while (x >= limit);
    return (uint8_t)(x % upper_exclusive);
}

static void generate_name(char out[6]) {
    out[0] = (char)('A' + get_uniform_u8(26));
    out[1] = (char)('A' + get_uniform_u8(26));
    out[2] = (char)('0' + get_uniform_u8(10));
    out[3] = (char)('0' + get_uniform_u8(10));
    out[4] = (char)('0' + get_uniform_u8(10));
    out[5] = '\0';
}

static int name_exists(const char *s) {
    for (size_t i = 0; i < allocated_count; i++) {
        if (allocated_names[i] && strcmp(allocated_names[i], s) == 0) return 1;
    }
    return 0;
}

static void add_name(const char *s) {
    if (allocated_count == allocated_capacity) {
        size_t newcap = allocated_capacity ? allocated_capacity * 2 : 64;
        char **tmp = (char**)realloc(allocated_names, newcap * sizeof(char*));
        if (!tmp) exit(1);
        allocated_names = tmp;
        for (size_t i = allocated_capacity; i < newcap; i++) allocated_names[i] = NULL;
        allocated_capacity = newcap;
    }
    char *copy = (char*)malloc(6);
    if (!copy) exit(1);
    memcpy(copy, s, 6);
    allocated_names[allocated_count++] = copy;
}

static void new_unique_name(char out[6]) {
    // There are 676000 possible names
    for (int attempts = 0; attempts < 700000; attempts++) {
        char candidate[6];
        generate_name(candidate);
        if (!name_exists(candidate)) {
            add_name(candidate);
            memcpy(out, candidate, 6);
            return;
        }
    }
    // If exhausted
    fprintf(stderr, "Exhausted all possible unique names\n");
    exit(1);
}

void robot_init(Robot *r) {
    if (!r) return;
    r->has_name = 0;
    r->name[0] = '\0';
}

const char* robot_get_name(Robot *r) {
    if (!r) return NULL;
    if (!r->has_name) {
        new_unique_name(r->name);
        r->has_name = 1;
    }
    return r->name;
}

void robot_reset(Robot *r) {
    if (!r) return;
    r->has_name = 0;
    // Old name remains reserved to avoid reuse
}

int main(void) {
    Robot r1, r2, r3, r4, r5;
    robot_init(&r1);
    robot_init(&r2);
    robot_init(&r3);
    robot_init(&r4);
    robot_init(&r5);

    const char* n1 = robot_get_name(&r1);
    const char* n2 = robot_get_name(&r2);
    const char* n3 = robot_get_name(&r3);
    printf("%s\n", n1);
    printf("%s\n", n2);
    printf("%s\n", n3);

    char old2[6];
    memcpy(old2, n2, 6);
    robot_reset(&r2);
    const char* n2b = robot_get_name(&r2);
    printf("%s\n", n2b);

    const char* n4 = robot_get_name(&r4);
    const char* n5 = robot_get_name(&r5);
    printf("%s\n", n4);
    printf("%s\n", n5);

    // Verify uniqueness of the five names after reset
    const char* names[5] = { n1, n2b, n3, n4, n5 };
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (strcmp(names[i], names[j]) == 0) {
                fprintf(stderr, "Names are not unique\n");
                return 1;
            }
        }
    }
    if (strcmp(old2, n2b) == 0) {
        fprintf(stderr, "Reset did not change the name\n");
        return 1;
    }
    return 0;
}