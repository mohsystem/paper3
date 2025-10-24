#include <openssl/rand.h>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <string>
#include <limits>

static const std::string ALPHANUM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
static const size_t MAX_BYTES = 1048576; // 1 MiB safety cap

std::vector<uint8_t> randomBytes(size_t length) {
    if (length > MAX_BYTES) {
        throw std::invalid_argument("length out of allowed range");
    }
    std::vector<uint8_t> buf(length);
    if (length > 0) {
        if (RAND_bytes(buf.data(), static_cast<int>(buf.size())) != 1) {
            throw std::runtime_error("RAND_bytes failed");
        }
    }
    return buf;
}

std::string toHex(const std::vector<uint8_t>& data) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(data.size() * 2);
    for (size_t i = 0; i < data.size(); ++i) {
        out[2 * i] = hex[(data[i] >> 4) & 0xF];
        out[2 * i + 1] = hex[data[i] & 0xF];
    }
    return out;
}

std::string randomHex(size_t nBytes) {
    return toHex(randomBytes(nBytes));
}

uint32_t randU32() {
    uint32_t v = 0;
    if (RAND_bytes(reinterpret_cast<unsigned char*>(&v), sizeof(v)) != 1) {
        throw std::runtime_error("RAND_bytes failed");
    }
    return v;
}

size_t secureIndex(size_t bound) {
    if (bound == 0) return 0;
    // Rejection sampling using 32-bit space
    const uint64_t space = (uint64_t{1} << 32);
    const uint64_t limit = (space / bound) * bound; // largest multiple <= 2^32
    while (true) {
        uint64_t v = randU32();
        if (v < limit) {
            return static_cast<size_t>(v % bound);
        }
    }
}

std::string randomAlphaNum(size_t length) {
    if (length > MAX_BYTES) {
        throw std::invalid_argument("length out of allowed range");
    }
    std::string s;
    s.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        size_t idx = secureIndex(ALPHANUM.size());
        s.push_back(ALPHANUM[idx]);
    }
    return s;
}

int randomIntInRange(int min, int max) {
    if (min == std::numeric_limits<int>::min() && max == std::numeric_limits<int>::max()) {
        int32_t r = 0;
        if (RAND_bytes(reinterpret_cast<unsigned char*>(&r), sizeof(r)) != 1) {
            throw std::runtime_error("RAND_bytes failed");
        }
        return r;
    }
    if (min > max) {
        throw std::invalid_argument("min must be <= max");
    }
    uint64_t range = static_cast<uint64_t>(static_cast<int64_t>(max) - static_cast<int64_t>(min)) + 1ULL; // 1..2^32
    const uint64_t space = (uint64_t{1} << 32);
    const uint64_t limit = (space / range) * range;
    while (true) {
        uint64_t v = randU32();
        if (v < limit) {
            return static_cast<int>(static_cast<int64_t>(min) + static_cast<int64_t>(v % range));
        }
    }
}

int main() {
    try {
        // 5 test cases
        std::cout << "Random hex (16 bytes): " << randomHex(16) << "\n";
        std::cout << "Random hex (32 bytes): " << randomHex(32) << "\n";
        std::cout << "Random alphanum (20 chars): " << randomAlphaNum(20) << "\n";
        std::cout << "Random int [0,10]: " << randomIntInRange(0, 10) << "\n";
        std::cout << "Random int full range: " << randomIntInRange(std::numeric_limits<int>::min(),
                                                                  std::numeric_limits<int>::max()) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}