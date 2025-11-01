#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cctype>

long long add1000(long long value) {
    return value + 1000LL;
}

bool isValidIntegerString(const std::string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    if (i >= s.size()) return false;
    if (s[i] == '+' || s[i] == '-') ++i;
    size_t digits = 0;
    for (; i < s.size(); ++i) {
        if (std::isspace(static_cast<unsigned char>(s[i]))) {
            // trailing spaces not allowed
            return false;
        }
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
        ++digits;
        if (digits > 19) return false;
    }
    return digits >= 1;
}

bool parseInt64(const std::string& s, long long& out) {
    if (!isValidIntegerString(s)) return false;
    errno = 0;
    char* endptr = nullptr;
    const char* cstr = s.c_str();
    long long val = std::strtoll(cstr, &endptr, 10);
    if (errno == ERANGE || endptr == cstr || *endptr != '\0') return false;
    out = val;
    return true;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string input(argv[1] ? argv[1] : "");
        long long n = 0;
        if (!parseInt64(input, n)) {
            std::cerr << "Invalid input\n";
            return 1;
        }
        if (n > LLONG_MAX - 1000LL) {
            std::cerr << "Invalid input\n";
            return 1;
        }
        long long result = add1000(n);
        std::cout << result << "\n";
        return 0;
    } else {
        // 5 test cases
        long long tests[5] = {
            0LL,
            1LL,
            -1000LL,
            123456789012345LL,
            LLONG_MAX - 1000LL
        };
        for (long long t : tests) {
            if (t > LLONG_MAX - 1000LL) {
                std::cout << "overflow\n";
            } else {
                std::cout << add1000(t) << "\n";
            }
        }
        return 0;
    }
}