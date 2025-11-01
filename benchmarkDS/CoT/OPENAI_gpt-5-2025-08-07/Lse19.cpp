// Step 1: Problem understanding:
// - Read a number from the command line, add 1000, and output the result.
// - Provide add1000 function that accepts input and returns output.

// Step 2: Security requirements:
// - Strictly validate input (no partial parses).
// - Check for overflow/underflow before addition.

// Step 3: Secure coding generation:
// - Use std::stoll with position tracking.
// - Use limits to check overflow.

// Step 4: Code review:
// - Ensure no undefined behavior on overflow.
// - Handle exceptions and invalid inputs gracefully.

// Step 5: Secure code output:
#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

static bool parse_long_long_strict(const std::string& s, long long& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    try {
        size_t pos = 0;
        long long v = std::stoll(t, &pos, 10);
        if (pos != t.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

long long add1000(long long value) {
    if (value > std::numeric_limits<long long>::max() - 1000LL ||
        value < std::numeric_limits<long long>::min() + 1000LL) {
        throw std::overflow_error("Overflow when adding 1000");
    }
    return value + 1000LL;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        long long v = 0;
        if (!parse_long_long_strict(argv[1], v)) {
            std::cerr << "Invalid integer input.\n";
            return 1;
        }
        try {
            std::cout << add1000(v) << "\n";
        } catch (const std::overflow_error&) {
            std::cerr << "Overflow detected.\n";
            return 1;
        }
    } else {
        long long tests[5] = {0LL, 42LL, -42LL,
                              std::numeric_limits<long long>::max() - 1000LL,
                              std::numeric_limits<long long>::min() + 1000LL};
        for (long long t : tests) {
            try {
                std::cout << "add1000(" << t << ") = " << add1000(t) << "\n";
            } catch (const std::overflow_error&) {
                std::cout << "add1000(" << t << ") = overflow\n";
            }
        }
    }
    return 0;
}