#include <iostream>
#include <limits>
#include <cerrno>
#include <cstdlib>
#include <cstring>

long long addThousand(long long x) {
    return x + 1000;
}

static bool parseLongLong(const char* s, long long& out) {
    if (s == nullptr) return false;
    errno = 0;
    char* end = nullptr;
    long long val = std::strtoll(s, &end, 10);
    if (errno == ERANGE || end == s || *end != '\0') {
        return false;
    }
    out = val;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        long long v = 0;
        if (parseLongLong(argv[1], v)) {
            if (v <= std::numeric_limits<long long>::max() - 1000 &&
                v >= std::numeric_limits<long long>::min() + 1000) {
                std::cout << addThousand(v) << '\n';
            } else {
                std::cerr << "Overflow risk when adding 1000.\n";
            }
        } else {
            std::cerr << "Invalid integer input.\n";
        }
    } else {
        long long tests[5] = {-1000, -1, 0, 1, 9223372036854774800LL};
        for (int i = 0; i < 5; ++i) {
            long long v = tests[i];
            if (v <= std::numeric_limits<long long>::max() - 1000 &&
                v >= std::numeric_limits<long long>::min() + 1000) {
                std::cout << addThousand(v) << '\n';
            } else {
                std::cout << "Overflow risk\n";
            }
        }
    }
    return 0;
}