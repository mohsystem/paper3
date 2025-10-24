#include <stdio.h>
#include <stdint.h>

/*
 Define a struct with bit fields representing a 16-bit status register:
 [bit0=isActive][bit1=isError][bits2-4=priority(3)][bits5-8=level(4)][bits9-15=code(7)]
*/
typedef struct {
    unsigned int isActive : 1;
    unsigned int isError  : 1;
    unsigned int priority : 3;
    unsigned int level    : 4;
    unsigned int code     : 7;
} StatusBits;

typedef struct {
    int ok;
    uint16_t value;
} PackResult;

typedef struct {
    int ok;
    StatusBits bits;
    uint16_t raw;
} UnpackResult;

enum {
    ACTIVE_SHIFT = 0,
    ERROR_SHIFT = 1,
    PRIORITY_SHIFT = 2,
    LEVEL_SHIFT = 5,
    CODE_SHIFT = 9
};

#define ACTIVE_MASK   ((uint16_t)(0x1u << ACTIVE_SHIFT))
#define ERROR_MASK    ((uint16_t)(0x1u << ERROR_SHIFT))
#define PRIORITY_MASK ((uint16_t)(0x7u << PRIORITY_SHIFT))
#define LEVEL_MASK    ((uint16_t)(0xFu << LEVEL_SHIFT))
#define CODE_MASK     ((uint16_t)(0x7Fu << CODE_SHIFT))

#define MAX_PRIORITY 7u
#define MAX_LEVEL    15u
#define MAX_CODE     127u

static int inRange(unsigned v, unsigned minv, unsigned maxv) {
    return (v >= minv) && (v <= maxv);
}

PackResult packStatus(unsigned isActive, unsigned isError, unsigned priority, unsigned level, unsigned code) {
    PackResult r;
    if (!inRange(isActive, 0u, 1u) || !inRange(isError, 0u, 1u) ||
        !inRange(priority, 0u, MAX_PRIORITY) ||
        !inRange(level, 0u, MAX_LEVEL) ||
        !inRange(code, 0u, MAX_CODE)) {
        r.ok = 0;
        r.value = 0;
        return r;
    }
    uint16_t v = 0;
    v |= (uint16_t)((isActive & 0x1u) << ACTIVE_SHIFT);
    v |= (uint16_t)((isError  & 0x1u) << ERROR_SHIFT);
    v |= (uint16_t)((priority & 0x7u) << PRIORITY_SHIFT);
    v |= (uint16_t)((level    & 0xFu) << LEVEL_SHIFT);
    v |= (uint16_t)((code     & 0x7Fu) << CODE_SHIFT);
    r.ok = 1;
    r.value = v;
    return r;
}

UnpackResult unpackStatus(uint16_t packed) {
    UnpackResult ur;
    ur.ok = 1;
    ur.raw = packed;
    StatusBits b;
    b.isActive = (unsigned)((packed & ACTIVE_MASK) >> ACTIVE_SHIFT);
    b.isError  = (unsigned)((packed & ERROR_MASK)  >> ERROR_SHIFT);
    b.priority = (unsigned)((packed & PRIORITY_MASK) >> PRIORITY_SHIFT);
    b.level    = (unsigned)((packed & LEVEL_MASK)    >> LEVEL_SHIFT);
    b.code     = (unsigned)((packed & CODE_MASK)     >> CODE_SHIFT);
    ur.bits = b;
    return ur;
}

static void printStatusBits(const StatusBits* b) {
    if (b == NULL) return;
    printf("StatusBits{isActive=%u, isError=%u, priority=%u, level=%u, code=%u}\n",
           (unsigned)b->isActive,
           (unsigned)b->isError,
           (unsigned)b->priority,
           (unsigned)b->level,
           (unsigned)b->code);
}

int main(void) {
    unsigned tests[5][5] = {
        {1u, 0u, 3u, 8u, 42u},
        {0u, 1u, 7u, 15u, 127u},
        {1u, 1u, 0u, 0u, 0u},
        {0u, 0u, 5u, 9u, 100u},
        {1u, 0u, 9u, 2u, 5u} /* invalid priority */
    };

    for (int i = 0; i < 5; i++) {
        unsigned* t = tests[i];
        printf("Test %d input: [%u, %u, %u, %u, %u]\n", i + 1, t[0], t[1], t[2], t[3], t[4]);
        PackResult pr = packStatus(t[0], t[1], t[2], t[3], t[4]);
        if (!pr.ok) {
            printf("Error: invalid parameters\n");
            continue;
        }
        printf("Packed: %u\n", (unsigned)pr.value);
        UnpackResult ur = unpackStatus(pr.value);
        printf("Unpacked: ");
        printStatusBits(&ur.bits);
    }
    return 0;
}