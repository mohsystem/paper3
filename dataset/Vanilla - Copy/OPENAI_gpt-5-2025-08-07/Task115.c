#include <stdio.h>
#include <stdint.h>

// Define a struct with bit fields
typedef struct {
    unsigned int version  : 3;  // 0-7
    unsigned int type     : 5;  // 0-31
    unsigned int flags    : 4;  // 0-15
    unsigned int length   : 12; // 0-4095
    unsigned int checksum : 8;  // 0-255
} PacketBF;

// Packs fields into a 32-bit word: [checksum:8][length:12][flags:4][type:5][version:3]
uint32_t pack_packet(unsigned int version, unsigned int type, unsigned int flags, unsigned int length, unsigned int checksum) {
    uint32_t v = version & 0x7u;
    uint32_t t = type & 0x1Fu;
    uint32_t f = flags & 0xFu;
    uint32_t l = length & 0xFFFu;
    uint32_t c = checksum & 0xFFu;
    return (c << 24) | (l << 12) | (f << 8) | (t << 3) | v;
}

// Unpacks a 32-bit word into a PacketBF struct
PacketBF unpack_packet_to_struct(uint32_t value) {
    PacketBF p;
    p.version  =  value        & 0x7u;
    p.type     = (value >> 3 ) & 0x1Fu;
    p.flags    = (value >> 8 ) & 0xFu;
    p.length   = (value >> 12) & 0xFFFu;
    p.checksum = (value >> 24) & 0xFFu;
    return p;
}

void print_packet(const PacketBF* p) {
    printf("Packet{version=%u, type=%u, flags=%u, length=%u, checksum=%u}", p->version, p->type, p->flags, p->length, p->checksum);
}

int main(void) {
    unsigned int tests[5][5] = {
        {1, 3, 2, 100, 200},
        {7, 31, 15, 4095, 255},
        {0, 0, 0, 0, 0},
        {5, 10, 8, 2048, 128},
        {2, 18, 7, 1234, 77}
    };

    for (int i = 0; i < 5; ++i) {
        unsigned int v = tests[i][0];
        unsigned int t = tests[i][1];
        unsigned int f = tests[i][2];
        unsigned int l = tests[i][3];
        unsigned int c = tests[i][4];

        uint32_t packed = pack_packet(v, t, f, l, c);
        PacketBF unpacked = unpack_packet_to_struct(packed);

        printf("Input  : (%u, %u, %u, %u, %u)\n", v, t, f, l, c);
        printf("Packed : 0x%08X\n", packed);
        printf("Unpack : ");
        print_packet(&unpacked);
        printf("\n---\n");
    }

    return 0;
}