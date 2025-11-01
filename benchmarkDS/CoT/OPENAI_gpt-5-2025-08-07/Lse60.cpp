#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <limits>

static const size_t MAX_SIZE = 1048576; // 1 MiB upper bound for safety

// Allocates a zero-initialized buffer of the given size and returns its contents as a hex string
std::string allocateAndGetHex(size_t size) {
    if (size > MAX_SIZE) {
        throw std::invalid_argument("Requested size exceeds safe limit.");
    }
    std::vector<unsigned char> buffer(size, 0); // zero-initialized
    std::string out;
    if (size > 0 && size > (std::numeric_limits<size_t>::max() / 2)) {
        throw std::overflow_error("Size too large to format.");
    }
    out.reserve(size * 2);
    static const char* HEX = "0123456789abcdef";
    for (unsigned char b : buffer) {
        out.push_back(HEX[(b >> 4) & 0xF]);
        out.push_back(HEX[b & 0xF]);
    }
    return out;
}

static bool parseSizeArg(const char* arg, size_t& out) {
    try {
        std::string s(arg);
        if (s.empty()) return false;
        size_t idx = 0;
        unsigned long long v = std::stoull(s, &idx, 10);
        if (idx != s.size()) return false;
        out = static_cast<size_t>(v);
        return true;
    } catch (...) {
        return false;
    }
}

int main(int argc, char** argv) {
    if (argc == 2) {
        size_t size = 0;
        if (!parseSizeArg(argv[1], size)) {
            std::cerr << "Invalid size argument.\n";
            return 1;
        }
        try {
            std::string out = allocateAndGetHex(size);
            std::cout << out << "\n";
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            return 1;
        }
        return 0;
    }
    // 5 test cases if no argument is provided
    size_t tests[5] = {0, 1, 8, 16, 32};
    for (size_t t : tests) {
        try {
            std::string out = allocateAndGetHex(t);
            std::cout << out << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Test case failed for size " << t << ": " << e.what() << "\n";
        }
    }
    return 0;
}