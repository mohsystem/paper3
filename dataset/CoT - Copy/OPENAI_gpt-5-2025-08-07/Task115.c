/* Chain-of-Through secure code generation process embedded as comments.
 * 1) Problem understanding: Define a C struct with bit fields for compact permissions.
 *    Fields: read(1), write(1), execute(1), role(2), level(5).
 * 2) Security requirements: Mask inputs, avoid layout-dependent packing by manual shifts,
 *    use fixed-width types and safe operations.
 * 3) Secure coding generation: Provide creation, pack, and unpack functions.
 * 4) Code review: All masks applied, no external input parsing.
 * 5) Secure code output: Final code with 5 test cases.
 */

#include <stdio.h>
#include <stdint.h>

typedef struct {
    unsigned int read    : 1;
    unsigned int write   : 1;
    unsigned int execute : 1;
    unsigned int role    : 2;
    unsigned int level   : 5;
} Permissions;

// Construct Permissions with masking to valid ranges.
Permissions make_permissions(unsigned int read, unsigned int write, unsigned int execute,
                             unsigned int role, unsigned int level) {
    Permissions p;
    p.read    = read   & 0x1;
    p.write   = write  & 0x1;
    p.execute = execute& 0x1;
    p.role    = role   & 0x3;
    p.level   = level  & 0x1F;
    return p;
}

// Pack Permissions into a 16-bit value using explicit shifts/masks.
uint16_t pack_permissions(Permissions p) {
    uint16_t v = 0;
    v |= ((uint16_t)(p.read)    & 0x1)  << 0;
    v |= ((uint16_t)(p.write)   & 0x1)  << 1;
    v |= ((uint16_t)(p.execute) & 0x1)  << 2;
    v |= ((uint16_t)(p.role)    & 0x3)  << 3;
    v |= ((uint16_t)(p.level)   & 0x1F) << 5;
    return v;
}

// Unpack a 16-bit value into Permissions by extracting fields.
Permissions unpack_permissions(uint16_t packed) {
    return make_permissions(
        (packed >> 0) & 0x1,
        (packed >> 1) & 0x1,
        (packed >> 2) & 0x1,
        (packed >> 3) & 0x3,
        (packed >> 5) & 0x1F
    );
}

// Helper to print Permissions.
void print_permissions(const char* label, Permissions p, uint16_t packed) {
    printf("%sPermissions(read=%u, write=%u, execute=%u, role=%u, level=%u, packed=0x%04X)\n",
           label ? label : "",
           p.read, p.write, p.execute, p.role, p.level, (unsigned)packed);
}

int main(void) {
    unsigned tests[5][5] = {
        {1,0,1,2,17},
        {0,1,1,1,31},
        {1,1,1,3,0},
        {0,0,0,0,5},
        {1,1,0,2,29}
    };

    for (int i = 0; i < 5; ++i) {
        Permissions p = make_permissions(tests[i][0], tests[i][1], tests[i][2], tests[i][3], tests[i][4]);
        uint16_t packed = pack_permissions(p);
        printf("Test %d -> ", i+1);
        print_permissions("", p, packed);

        Permissions u = unpack_permissions(packed);
        uint16_t repacked = pack_permissions(u);
        printf("Unpacked %d -> ", i+1);
        print_permissions("", u, repacked);
    }
    return 0;
}