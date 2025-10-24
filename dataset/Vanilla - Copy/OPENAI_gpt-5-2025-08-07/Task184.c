#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TOTAL_NAMES (26 * 26 * 1000)

static unsigned char allocated[TOTAL_NAMES] = {0};
static int factory_initialized = 0;

static void init_factory(void) {
    if (!factory_initialized) {
        unsigned int seed = (unsigned int)(time(NULL) ^ (uintptr_t)&seed ^ (uintptr_t)allocated);
        srand(seed);
        factory_initialized = 1;
    }
}

static int index_from_name(const char* name) {
    if (!name || strlen(name) != 5) return -1;
    int l1 = name[0] - 'A';
    int l2 = name[1] - 'A';
    if (l1 < 0 || l1 >= 26 || l2 < 0 || l2 >= 26) return -1;
    int num = (name[2] - '0') * 100 + (name[3] - '0') * 10 + (name[4] - '0');
    if (num < 0 || num > 999) return -1;
    return ((l1 * 26) + l2) * 1000 + num;
}

static void name_from_index(int idx, char out[6]) {
    int l1 = idx / (26 * 1000);
    int r = idx % (26 * 1000);
    int l2 = r / 1000;
    int num = r % 1000;
    out[0] = (char)('A' + l1);
    out[1] = (char)('A' + l2);
    out[2] = (char)('0' + (num / 100) % 10);
    out[3] = (char)('0' + (num / 10) % 10);
    out[4] = (char)('0' + (num % 10));
    out[5] = '\0';
}

static int generate_unique_name(char out[6]) {
    init_factory();
    static int allocated_count = 0;
    if (allocated_count >= TOTAL_NAMES) {
        return -1; // exhausted
    }
    int start = rand() % TOTAL_NAMES;
    int idx = start;
    do {
        if (!allocated[idx]) {
            allocated[idx] = 1;
            allocated_count++;
            name_from_index(idx, out);
            return 0;
        }
        idx++;
        if (idx >= TOTAL_NAMES) idx = 0;
    } while (idx != start);
    return -1;
}

static void release_name(const char* name) {
    int idx = index_from_name(name);
    if (idx >= 0 && idx < TOTAL_NAMES) {
        allocated[idx] = 0;
    }
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
        if (generate_unique_name(r->name) != 0) {
            return NULL; // exhausted
        }
        r->has_name = 1;
    }
    return r->name;
}

static void robot_reset(Robot* r) {
    if (!r) return;
    if (r->has_name) {
        release_name(r->name);
        r->has_name = 0;
        r->name[0] = '\0';
    }
}

int main(void) {
    Robot r1, r2, r3, r4;
    robot_init(&r1);
    robot_init(&r2);
    robot_init(&r3);
    robot_init(&r4);

    printf("R1: %s\n", robot_get_name(&r1));
    printf("R2: %s\n", robot_get_name(&r2));
    printf("R3: %s\n", robot_get_name(&r3));
    printf("R4: %s\n", robot_get_name(&r4));
    robot_reset(&r2);
    printf("R2 after reset: %s\n", robot_get_name(&r2));

    return 0;
}