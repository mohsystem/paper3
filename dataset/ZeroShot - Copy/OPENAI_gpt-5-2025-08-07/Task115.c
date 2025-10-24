#include <stdio.h>
#include <stdint.h>
#include <limits.h>

struct FlagsBF {
    unsigned int read : 1;
    unsigned int write : 1;
    unsigned int execute : 1;
    unsigned int role : 2;      /* 0-3 */
    unsigned int priority : 4;  /* 0-15 */
};

static int validate(unsigned int r, unsigned int w, unsigned int x, unsigned int role, unsigned int prio) {
    if (r > 1U || w > 1U || x > 1U) return 0;
    if (role > 3U) return 0;
    if (prio > 15U) return 0;
    return 1;
}

uint32_t build_packed(unsigned int r, unsigned int w, unsigned int x, unsigned int role, unsigned int prio) {
    if (!validate(r, w, x, role, prio)) {
        return UINT32_MAX;
    }
    uint32_t p = 0U;
    p |= (r & 1U) << 0;
    p |= (w & 1U) << 1;
    p |= (x & 1U) << 2;
    p |= (role & 3U) << 3;
    p |= (prio & 0xFU) << 5;
    return p;
}

struct FlagsBF unpack_to_struct(uint32_t packed) {
    struct FlagsBF f;
    f.read = (packed >> 0) & 1U;
    f.write = (packed >> 1) & 1U;
    f.execute = (packed >> 2) & 1U;
    f.role = (packed >> 3) & 3U;
    f.priority = (packed >> 5) & 0xFU;
    return f;
}

int main(void) {
    unsigned int tests[5][5] = {
        {1U, 1U, 0U, 2U, 7U},
        {0U, 0U, 1U, 1U, 0U},
        {1U, 0U, 1U, 3U, 15U},
        {1U, 1U, 1U, 4U, 2U},   /* invalid role */
        {1U, 0U, 0U, 0U, 16U}   /* invalid priority */
    };
    for (int i = 0; i < 5; ++i) {
        uint32_t packed = build_packed(tests[i][0], tests[i][1], tests[i][2], tests[i][3], tests[i][4]);
        if (packed == UINT32_MAX) {
            printf("Test %d: error=invalid input\n", i + 1);
        } else {
            struct FlagsBF f = unpack_to_struct(packed);
            printf("Test %d: packed=%u unpacked={read=%u, write=%u, execute=%u, role=%u, priority=%u}\n",
                   i + 1, packed, f.read, f.write, f.execute, f.role, f.priority);
        }
    }
    return 0;
}