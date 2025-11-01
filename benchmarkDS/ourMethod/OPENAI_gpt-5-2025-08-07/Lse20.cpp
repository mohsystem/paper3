#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>

static const long long OFFSET = 1000LL;

long long addOffset(long long value) {
    if (value > LLONG_MAX - OFFSET) {
        throw std::overflow_error("overflow");
    }
    return value + OFFSET;
}

static bool tryParseI64(const std::string& s, long long& out) {
    std::string t;
    // Trim spaces
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    if (start == std::string::npos) return false;
    t = s.substr(start, end - start + 1);
    if (t.empty() || t.size() > 20) return false; // sign + up to 19 digits

    // Basic format check
    size_t i = 0;
    if (t[0] == '+' || t[0] == '-') {
        if (t.size() == 1) return false;
        i = 1;
    }
    for (; i < t.size(); ++i) {
        if (t[i] < '0' || t[i] > '9') return false;
    }

    errno = 0;
    char* endptr = nullptr;
    const char* cstr = t.c_str();
    long long v = std::strtoll(cstr, &endptr, 10);
    if (errno == ERANGE || endptr == cstr || *endptr != '\0') {
        return false;
    }
    out = v;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        long long input = 0;
        if (!tryParseI64(std::string(argv[1]), input)) {
            std::cerr << "Invalid input" << std::endl;
            return 1;
        }
        try {
            long long result = addOffset(input);
            std::cout << result << std::endl;
        } catch (const std::overflow_error&) {
            std::cerr << "Overflow" << std::endl;
            return 1;
        }
    } else {
        // 5 test cases
        long long tests[5] = {0LL, 42LL, -1000LL, LLONG_MAX - OFFSET, LLONG_MIN + OFFSET};
        for (long long v : tests) {
            try {
                std::cout << addOffset(v) << std::endl;
            } catch (const std::overflow_error&) {
                std::cout << "Overflow" << std::endl;
            }
        }
    }
    return 0;
}