#include <iostream>
#include <iomanip>
#include <tuple>
#include <string>
#include <vector>
#include <cstdint>

// Define a struct with bit fields
struct PacketBF {
    unsigned int version : 3;  // 0-7
    unsigned int type    : 5;  // 0-31
    unsigned int flags   : 4;  // 0-15
    unsigned int length  : 12; // 0-4095
    unsigned int checksum: 8;  // 0-255
};

// Packs fields into a 32-bit word: [checksum:8][length:12][flags:4][type:5][version:3]
uint32_t pack_packet(unsigned int version, unsigned int type, unsigned int flags, unsigned int length, unsigned int checksum) {
    uint32_t v = version & 0x7u;
    uint32_t t = type & 0x1Fu;
    uint32_t f = flags & 0xFu;
    uint32_t l = length & 0xFFFu;
    uint32_t c = checksum & 0xFFu;
    return (c << 24) | (l << 12) | (f << 8) | (t << 3) | v;
}

// Unpacks a 32-bit word into a PacketBF struct (assigning fields)
PacketBF unpack_packet_to_struct(uint32_t value) {
    PacketBF p{};
    p.version  = (value      ) & 0x7u;
    p.type     = (value >> 3 ) & 0x1Fu;
    p.flags    = (value >> 8 ) & 0xFu;
    p.length   = (value >> 12) & 0xFFFu;
    p.checksum = (value >> 24) & 0xFFu;
    return p;
}

std::string packet_to_string(const PacketBF& p) {
    return "Packet{version=" + std::to_string(p.version) +
           ", type=" + std::to_string(p.type) +
           ", flags=" + std::to_string(p.flags) +
           ", length=" + std::to_string(p.length) +
           ", checksum=" + std::to_string(p.checksum) + "}";
}

int main() {
    std::vector<std::tuple<unsigned,int,unsigned,int,unsigned>> tests = {
        {1, 3, 2, 100, 200},
        {7, 31, 15, 4095, 255},
        {0, 0, 0, 0, 0},
        {5, 10, 8, 2048, 128},
        {2, 18, 7, 1234, 77}
    };

    for (auto& t : tests) {
        unsigned v, ty, f, l, c;
        std::tie(v, ty, f, l, c) = t;
        uint32_t packed = pack_packet(v, ty, f, l, c);
        PacketBF unpacked = unpack_packet_to_struct(packed);

        std::cout << "Input  : (" << v << ", " << ty << ", " << f << ", " << l << ", " << c << ")\n";
        std::cout << "Packed : 0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << packed << std::dec << "\n";
        std::cout << "Unpack : " << packet_to_string(unpacked) << "\n";
        std::cout << "---\n";
    }
    return 0;
}