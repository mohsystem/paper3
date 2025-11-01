#include <iostream>
#include <limits>
#include <stdexcept>
#include <cstdlib>
#include <cerrno>

static const long long OFFSET = 1000LL;

long long addOffset(long long value) {
    if (value > std::numeric_limits<long long>::max() - OFFSET) {
        throw std::overflow_error("Overflow when adding offset");
    }
    if (value < std::numeric_limits<long long>::min() + OFFSET) {
        throw std::overflow_error("Underflow when adding offset");
    }
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        errno = 0;
        char* end = nullptr;
        const char* s = argv[1];
        long long v = std::strtoll(s, &end, 10);
        if (errno == ERANGE || end == s || *end != '\0') {
            std::cerr << "Error: invalid integer input\n";
            return 1;
        }
        try {
            long long res = addOffset(v);
            std::cout << res << "\n";
        } catch (const std::overflow_error& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else {
        long long tests[5] = {
            0LL,
            1LL,
            -1LL,
            std::numeric_limits<long long>::max() - 1000LL,
            std::numeric_limits<long long>::min() + 1000LL
        };
        for (long long t : tests) {
            try {
                long long r = addOffset(t);
                std::cout << "addOffset(" << t << ") = " << r << "\n";
            } catch (const std::overflow_error& e) {
                std::cout << "addOffset(" << t << ") error: " << e.what() << "\n";
            }
        }
    }
    return 0;
}